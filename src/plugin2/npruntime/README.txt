The npruntime source code is taken from Seamonkey. http://mxr.mozilla.org/seamonkey/source/modules/plugin/samples/npruntime/

However, the latest Gecko SDK 8.0 for Firefox 8 is no more compatible with
the plugin code. So some modifications are done for MS VC++.

General issues can be found here:
https://developer.mozilla.org/en/Compiling_The_npruntime_Sample_Plugin_in_Visual_Studio

In addition to the issues listed in that page, more parts need to be fixed.
The following applies to MS VC++ .net 2003.
I'm not sure if they applies to newer versions.

1. PR_TRUE/PR_FALSE are replaced with true/false.

2. The integer types int32, uint32, int16, ...etc are appended with _t suffixs.

3. Manually #include "npfunctions.h" is required for np_entry.cpp and npn_gate.cpp since
   NPNetscapeFuncs is defined in npfunctions.h

4. Some code for testing purpose are removed from CPlugin constructor.

5. Some code for XP_MAC and XP_UNIX are removed since this is a Windows only plugin.

6. A really bad news is, VC++ does not support C99 and does not have 
   inttypes.h and stdint.h, which unfortunately defines the integer types used.
   Lukily, a compatible replacement can be found here:
   http://code.google.com/p/msinttypes/
   This adds missing the headers to MS VC++ and the plugin can build.

7. For better readability, I moved ScriptablePluginObject stuff to its own cpp file.
