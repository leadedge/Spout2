Set oShell = CreateObject ("Wscript.Shell") 
Dim strArgs
strArgs = "cmd /c aa-video.bat"
oShell.Run strArgs, 0, false


