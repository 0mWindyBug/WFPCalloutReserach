# WFPResearch
research revolving the windows filtering platform callout mechanism 



# Callout Enumeration 
a driver registers a callout with the filter engine using FwpsCalloutRegister

in addition , a driver has to add the callout to a layer using FwpmCalloutAdd

and create a filter that uses the callout , using FwpmFilterAdd

generally , a callout is registered with a GUID, and identified internally by the filter engine with a corresponding ID 

what woukd we like? a way to enumerate all registered callout routines on the system (this can be useful for debugging as well as offensive purposes such as silencing AVs callouts ) 

we would also like to gather as much information as possible about the callout identified in enumeration (ie which layer is it on? that way we can silence specific callouts )

two functions we might be able to take advantage of in the process are 
1. FwpmCalloutEnum(allowing us to enumerate all callout ids from usermode)
2. KfdGetCalloutRef (allowing us to get a callout struct pointer from callout id , undocumented export of netio.sys)
