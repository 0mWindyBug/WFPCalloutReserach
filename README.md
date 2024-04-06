# WFPResearch
short research revolving the windows filtering platform callout mechanism 

# Sources 
WFPEnum and WFPEnumDriver can be used to enumerate all registered callouts on the system (outputting their actual addresses, load the driver and run the client) 
WFPCalloutDriver is a PoC callout driver (mainly used it for debugging but you can have a look to see the registration process) 

# quick overview of the windows filtering platform 
if it's the first time you hear about WFP , I highly recommend you read https://scorpiosoftware.net/2022/12/25/introduction-to-the-windows-filtering-platform/ 

so , the windows filtering platform (WFP) is a framework designated for host-based network traffic filtering , replacing the older NDIS and TDI filtering capabilities 

WFP exposes both UM and KM apis , offering the ability to block , permit or aduit network tarffic based on conditions or deep packet inspection (through callouts)  

as you might have guessed , WFP can be (and is) used by the windows firewall, network filters of security software and even rootkits.

### layers, sublayers , filters and shims 
layers are used to categorize the network traffic to be evaluated , there are roughly a hundred layers (each is identified by a GUID) where filters and callouts can be attached and each represents a location the network processing path of a potential packet (for example you can attach on FWPM_LAYER_INBOUND_TRANSPORT_V4 which is  located in the receive path just after a received packet's transport header has been parsed by the network stack at the transport layer, but before any transport layer processing takes place) 

next we have filters , which are made up from conditions (source port , ip , application etc) and action (permit, block, callout unknown, callout terminating and callout inspection) 

when the action is a callout the filter engine will call the callout classify function whenever the filter conditions match.
a callout can return permit , block or continue (meaning the filter should be 'ignored') , if the action is callout terminating the callout should only return permit or block , if the it's inspection it should only return continue, unknown means the callout might terminate or not based on the result of the classification , 

a sublayer is essentially a way to logically group filters (say you filter TCP traffic , and want to have different filters for ports hight than 1000 and lower than 1000 , you can create two sublayers) , hopfully it'll be more clear in the next section 

lastly we have the shims , a kernel component which is responsible for starting the classification -> applying the correct filters, potentially callouts to , at the end , make a decision regarding allowing / blocking the packet , the shim is called by the tcpip driver when a packet arrives at the network stack (of course , for each layer it goes through) 

### Weight , Filter Arbitration and Policy 
filter arbitration is the logic built into the WFP that is used to define how filters work with each other when making network filtering decisions 

surely , as part of filter arbitration some ordering needs to be applied when assesing filters - that's where weight comes into play. 
each filter has an assocciated weight which defines it's priority within the sublayer , each sublayer has it's own assocciated weight value to define it's priority within the layer 

network traffic traverses sublayers from the one with the hightest weight (priority) to the lowest , the final decision is made after all sublayers have been evaluated , allowing a mulitple matching capability. 

within a subalyer , filter arbitration computes the list of matching filters ordered by weight and evaluates them in order until a filter returns permit or block (lower priority filter that havent been evaluated will be skipped ) or until the list is exausted. 

as mentioned , within a layer all sublayers are evaluated even if higher priority sublayer has decided to block / permit the traffic , the final decision is based on a well defined policy 

the basic policy is :
* actions are evaluated from high priotiy sublayer to lower priority sublayers
* a block decision overrides a permit decision
* a block decision is final , packet is discarded


# Enumerating Callouts  
the more complex and interesting network filtering and inspection logic is implemented through callouts , enumerating registered callouts (and their actual addresses) can be useful for anyone with the intention of silecing or manipulating them , or if you are a WFP driver developer -for debugging.  so where do we start ?  

a driver registers a callout with the filter engine using FwpsCalloutRegister , passing a structure that describes the callout to be registered 
```
typedef struct FWPS_CALLOUT0_ {
  GUID                                calloutKey;
  UINT32                              flags;
  FWPS_CALLOUT_CLASSIFY_FN0           classifyFn;
  FWPS_CALLOUT_NOTIFY_FN0             notifyFn;
  FWPS_CALLOUT_FLOW_DELETE_NOTIFY_FN0 flowDeleteFn;
} FWPS_CALLOUT0;
```
the classify function is where the actual filtering logic is present , notify function is called when a filter that references the callout is added or removed 
one more thing to note is a flag called FWP_CALLOUT_FLAG_CONDITIONAL_ON_FLOW  - A callout driver can specify this flag when registering a callout that will be added at a layer that supports data flows. If this flag is specified, the filter engine calls the callout driver's classifyFn0 callout function only if there is a context associated with the data flow. A callout driver associates a context with a data flow by calling the FwpsFlowAssociateContext0 function.

in addition , a driver has to add the callout to a layer on the system using FwpmCalloutAdd (can also be done from UM)

and create a filter that uses the callout , using FwpmFilterAdd (can also be done from UM) 

generally , a callout is registered with a GUID, and identified internally by the filter engine with a corresponding ID 

an example callout driver is provided in the sources to demonstrate the registration of a filter that uses a callout 

##  reversing the callout registration mechanism
as always with callouts (or 'callbacks) mechanisms , the registration function is a good starting point as it's likely at one point or another interact with how callouts are organised internally , reversing FwpsCalloutRegister we end up with the following sequence of calls : 

fwpkclnt!FwpsCalloutRegister<X> -> fwpkclnt!FwppCalloutRegister -> fwpkclnt!FwppCalloutRegister -> NETIO!KfdAddCalloutEntry -> NETIO!FeAddCalloutEntry

reversed code of NETIO!FeAddCalloutEntry is shown below 

```
__int64 __fastcall FeAddCalloutEntry(
        int a1,
        __int64 ClassifyFunction,
        __int64 NotifyFn,
        __int64 FlowDeleteFn,
        int Flags,
        char a6,
        unsigned int CalloutId,
        __int64 DeviceObject)
{
  __int64 v12; // rcx
  __int64 CalloutEntry; // rdi
  char v14; // bp
  __int64 CalloutEntryPtr; // rbx
  __int64 v16; // rax

  CalloutEntry = WfpAllocateCalloutEntry(CalloutId);
  if ( CalloutEntry )
    goto LABEL_17;
  v14 = 1;
  CalloutEntryPtr = *(_QWORD *)(gWfpGlobal + 0x198) + 0x50i64 * CalloutId;
  if ( !*(_DWORD *)(CalloutEntryPtr + 4) && !*(_DWORD *)(CalloutEntryPtr + 8) )
  {
LABEL_6:
    if ( !CalloutEntry )
      goto LABEL_7;
LABEL_17:
    WfpReportError(CalloutEntry, "FeAddCalloutEntry");
    return CalloutEntry;
  }
  v16 = WfpReportSysErrorAsNtStatus(v12, "IsCalloutEntryAvailable", 0x40000000i64, 1i64);
  CalloutEntry = v16;
  if ( v16 )
  {
    WfpReportError(v16, "IsCalloutEntryAvailable");
    goto LABEL_6;
  }
LABEL_7:
  memset(CalloutEntryPtr, 0i64, 0x50i64);
  *(_DWORD *)CalloutEntryPtr = a1;
  *(_DWORD *)(CalloutEntryPtr + 4) = 1;
  if ( a1 == 3 )
    *(_QWORD *)(CalloutEntryPtr + 40) = ClassifyFunction;
  else
    *(_QWORD *)(CalloutEntryPtr + 16) = ClassifyFunction;
  *(_DWORD *)(CalloutEntryPtr + 48) = Flags;
  *(_BYTE *)(CalloutEntryPtr + 73) = a6;
  *(_QWORD *)(CalloutEntryPtr + 24) = NotifyFn;
  *(_QWORD *)(CalloutEntryPtr + 32) = FlowDeleteFn;
  *(_BYTE *)(CalloutEntryPtr + 72) = 0;
  *(_WORD *)(CalloutEntryPtr + 74) = 0;
  *(_DWORD *)(CalloutEntryPtr + 76) = 0;
  if ( DeviceObject )
  {
    ObfReferenceObject(DeviceObject);
    *(_QWORD *)(CalloutEntryPtr + 64) = DeviceObject;
  }
  if ( !dword_1C007D018 || !(unsigned __int8)tlgKeywordOn(&dword_1C007D018, 2i64) )
    v14 = 0;
  if ( v14 )
    WfpCalloutDiagTraceCalloutAddOrRegister(CalloutId, CalloutEntryPtr);
  return CalloutEntry;
}
```
we can see our callout and all required information is stored in memory referenced by ( NETIO!gWfpGlobal + 0x198 ) * (CalloutId + 0x50) , in other words, NETIO!g_WfpGlobal + 0x198 (build specific offset) is an array of callout structures , each of size 0x50 (build specific size) , where at offset 0x10 we can find the ClassifyFunction 

messing around with other references to this offset , you'll find a function called NETIO!FeInitCalloutTable 
![FeInitCalloutTable](https://github.com/0mWindyBug/WFPResearch/assets/139051196/08545957-5dc3-4776-a0de-d99d52c9502a)

The default initial size of this memory(gWfpGlobal!0x198) is 0x14000 bytes. 
every time there is a WFP registration, this value can be expanded/modified as needed ->  memory will be re-applied, data copied, and then the original memory will be deleted.
also , as you can see gWfpGlobal+0x190 is initialized with 1024 , 1024 * 0x50 (entry size) = 0x14000 , meaning g_WfpGlobal+0x190 stores the max callout id in the array / number of entries. 

#### FeGetWfpGlobalPtr 
there's an exported function by NETIO that will return the address of gWfpGlobal
![getwfpglobalptr](https://github.com/0mWindyBug/WFPResearch/assets/139051196/707f581a-82fe-42bc-961b-85afe1887b48)


by now , we have enough knowledge to : 
* find the address of NETIO!gWfpGlobal (sig scan from UM or FeGetWfpGlobalPtr if you can load a driver)
* read offsets 0x198 and 0x190 to get the array pointer and the maximum number of entries
* traverse all entries , the address stored at offset 0x10 from each entry is the classify callout : )

whilst this is certianly an option , and it has actually been actually used in the wild (by Lazarus's FudModule rootkit) , it's not the most reliable approach we can take . 

### NETIO!KfdGetRefCallout 
there's a function called GetCalloutEntry in NETIO which is hard to not notice , reversed code below 
![GetCalloutEntry](https://github.com/0mWindyBug/WFPResearch/assets/139051196/f7155b25-4115-45ea-8176-3b3bcb4cb105)

even better ? there's an undocumented export called NETIO!KfdGetRefCallout which essentially wraps this GetCalloutEntry 
(KfdGetRefCallout -> FeGetRefCallout > GetCalloutEntry) , now , by callout id we can get a pointer to it's corresponding callout entry without relying on the gWfpGlobal offsets : ) 
![KfdGetREF](https://github.com/0mWindyBug/WFPResearch/assets/139051196/02e880ef-74d2-4202-9dff-0b765029c6a1)

( note : we have to call NETIO!KfdDeRefCallout for each call )

### FwpmCalloutEnum usermode API 
putting it all together , we can find all registered callout ids on the system with the FwpmCalloutEnum0 API from usermode 
the provided source WFPEnumDriver exposes an IOCTL that gets a callout id , and returns it's corresponding the CalloutEntry pointer , ClassifyFunction and NotifyFunction 
the usermode client WFPEnum leverages that IOCTL for each callout id enumerated by FwpmCalloutEnum and display all information (the addresses , name , layer guid etc...) about each registered callout 

running it we get the following output : ) 
![CalloutsOutput](https://github.com/0mWindyBug/WFPResearch/assets/139051196/6c8d8ddf-18ed-4fad-919f-48ef2af3580b)





 functions we might be able to take advantage of in the process are 
1. FwpmCalloutEnum(allowing us to enumerate all callout ids from usermode)
2. KfdGetRefCallout (allowing us to get a callout struct pointer from callout id , undocumented export of netio.sys), we must call KfdDeRefCallout to decrement the reference
3. FeGetWfpGlobalPtr (returns a pointer to WfpGlobal)

practically : 
__int64 __fastcall KfdGetRefCallout(__int64 CalloutId, _QWORD *CalloutEntry) 

it will call -> FeGetRefCalloutEx(CalloutId, 0, &LocCalloutEntry); 

which in turn calls 

GetCalloutEntryEx(CalloutId, Zero, CalloutEntryPtr);

which checks 
 if ( CalloutId >= *(_DWORD *)(gWfpGlobal + 0x198) )

 and if the callout id is smaller 

 CalloutEntry = 0x60i64 * CalloutId + *(_QWORD *)(gWfpGlobal + 0x1A0);



The gWfpGlobal structure has a member called CalloutTable (named by yourself), which is a pointer. The default initial size of this memory is 0x14000 bytes.
Every time there is a WFP registration, this value will be expanded/modified: memory will be re-applied, data copied, and then the original memory will be deleted.


# Callout Registration 

fwpkclnt!FwpsCalloutRegister<X> -> fwpkclnt!FwppCalloutRegister -> ( fwpkclnt!FwppCalloutFindByKey) (there's also some global named gFwppCallouts)
fwpkclnt!FwppCalloutRegister -> NETIO!KfdAddCalloutEntry 

 NETIO!KfdAddCalloutEntry -> NETIO!FeAddCalloutEntry -> WfpAllocateCalloutEntry

## Callout Invocation 


## Silencing Callouts  
   
some question marks ?
1. which functiom is responsible for invoking callouts , have a short look at it
2. how can we call KfdGetRefCallout and KfdDeRefCallout? what is the prototype ? how can we call FeGetWfpGlobalPtr? 
3. are callout ids guranteed to be in acceding order ? send the max ID over IOCTL and for each get the callout structure ?
4. where is the callout layer stored ? when we register callout in the filter engine it gets added to the wfpglobal array? maybe the invocation process iterates over the layer's registered callout ids and finds the corresponding callout classify 
