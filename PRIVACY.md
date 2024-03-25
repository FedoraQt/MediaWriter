# Privacy information

AOSC Media Writer uses a custom User-Agent header for the AOSC project to have a clear statistic of how prevalent of a download method it is. 

This User-Agent string is in the following format: `AOSCMediaWriter/$VERSION ($OS $OSVERSION; $BUILDARCH; $LOCALE; $DETAILS)`.

You can disable this behavior by using `mediawriter --no-user-agent` which will make it use the Qt default User-Agent string (likely `Mozilla/5.0`).
