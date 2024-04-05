# WFPResearch
research revolving the windows filtering platform callout mechanism 



# Callout Enumeration 
a driver registers a callout with the filter engine using FwpsCalloutRegister

in addition , a driver has to add the callout to a layer using FwpmCalloutAdd

and create a filter that uses the callout , using FwpmFilterAdd

generally , a callout is registered with a GUID, and identified internally by the filter engine with a corresponding ID 

what woukd we like? a way to enumerate all registered callout routines on the system (this can be useful for debugging as well as offensive purposes such as silencing AVs callouts ) 

we would also like to gather as much information as possible about the callout identified in enumeration (ie which layer is it on? that way we can silence specific callouts )

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


# Callout Registration 

fwpkclnt!FwpsCalloutRegister<X> -> fwpkclnt!FwppCalloutRegister -> ( fwpkclnt!FwppCalloutFindByKey) (there's also some global named gFwppCallouts)
fwpkclnt!FwppCalloutRegister -> NETIO!KfdAddCalloutEntry 

 NETIO!KfdAddCalloutEntry -> NETIO!FeAddCalloutEntry

   
some question marks ?
1. which functiom is responsible for invoking callouts , have a short look at it
2. how can we call KfdGetRefCallout and KfdDeRefCallout? what is the prototype ? how can we call FeGetWfpGlobalPtr? 
3. are callout ids guranteed to be in acceding order ? send the max ID over IOCTL and for each get the callout structure ?
4. where is the callout layer stored ? when we register callout in the filter engine it gets added to the wfpglobal array? maybe the invocation process iterates over the layer's registered callout ids and finds the corresponding callout classify 
