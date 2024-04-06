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

## layers, sublayers , filters and shims 
layers are used to categorize the network traffic to be evaluated , there are roughly a hundred layers (each is identified by a GUID) where filters and callouts can be attached and each represents a location the network processing path of a potential packet (for example you can attach on FWPM_LAYER_INBOUND_TRANSPORT_V4 which is  located in the receive path just after a received packet's transport header has been parsed by the network stack at the transport layer, but before any transport layer processing takes place) 

next we have filters , which are made up from conditions (source port , ip , application etc) and action (permit, block, callout unknown, callout terminating and callout inspection) 

when the action is a callout the filter engine will call the callout classify function whenever the filter conditions match.
a callout can return permit , block or continue (meaning the filter should be 'ignored') , if the action is callout terminating the callout should only return permit or block , if the it's inspection it should only return continue, unknown means the callout might terminate or not based on the result of the classification , 

a sublayer is essentially a way to logically group filters (say you filter TCP traffic , and want to have different filters for ports hight than 1000 and lower than 1000 , you can create two sublayers) , hopfully it'll be more clear in the next section 

lastly we have the shims , a kernel component which is responsible for starting the classification -> applying the correct filters, potentially callouts to , at the end , make a decision regarding allowing / blocking the packet , the shim is called by the tcpip driver when a packet arrives at the network stack (of course , for each layer it goes through) 

## Weight , Filter Arbitration and Policy 
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

a driver registers a callout with the filter engine using FwpsCalloutRegister

in addition , a driver has to add the callout to a layer on the system using FwpmCalloutAdd (can also be done from UM)

and create a filter that uses the callout , using FwpmFilterAdd (can also be done from UM) 

generally , a callout is registered with a GUID, and identified internally by the filter engine with a corresponding ID 

an example callout driver is provided in the sources to demonstrate the registration of a filter that uses a callout 



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
