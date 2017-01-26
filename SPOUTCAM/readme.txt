SPOUTCAM

SpoutCam is a virtual webcam that is also a Spout receiver.

SpoutCam is registered by the installer, and there is no need to take any further action.

Not all applications support virtual webcams so if you have problems, uninstall Spout and re-install without the SpoutCam option selected.


SPOUTCAM SETTINGS

Previous versions of SpoutCam have always used use the resolution of the first sender running when it starts - as long as the sender is started before SpoutCam. Otherwise it has used the default 640x480.

But some host programs don't accept arbitrary resolutions, and expect typical webcam formats like 640x480. Also a frame rate of 30 fps is typical for webcams and some host programs might not accept SpoutCam when it is running at 60fps.

"SpoutCamSettings.exe" is a dialog program that allows the frame rate and resolution to be specified and SpoutCam will use these settings when it starts.

You only need to change these settings if you require compatibility for host programs, otherwise the defaults are the same as previous versions of SpoutCam - "Active sender" and "60fps".

For example if you are having trouble with a host program, it could be better to select "640x480" and "30fps" so that SpoutCam behaves more like a typical webcam.


RESOLUTION

The resolution actually used depends on :

  o What has been selected by SpoutCamSettings
  o Whether a sender is already running.

1) If a sender is running when SpoutCam starts, it will adapt to the sender resolution if "Active sender" has been selected as the starting resolution.

2) If any other fixed resolution has been selected, it will always be used whether the sender is started first or not.

3) If no sender is running when SpoutCam starts and "Active sender" has been selected, the resolution is undefined, so the default 640x480 is used.


FRAME RATE

You will get better performance by selecting 60fps if the host program accepts it, and if the CPU performance is sufficient for the processing required.

The frame rate set with "SpoutCamSettings" is the “desired” frame rate. SpoutCam will attempt to meet that frame rate and drop frames if it cannot keep up.

Some applications will not accept 60fps and might require 30fps to be selected.


SUMMARY

To get SpoutCam to adapt to the resolution of the sender, select "Active sender" in the settings and start the sender before starting SpoutCam. This is the most efficient way to operate because there is no image size conversion.

If you want to use a common webcam resolution for compatibility reasons, select it and SpoutCam will retain that resolution.

Use 30fps for best compatibility with webcams, but use 60fps for better performance if it is accepted by the host program.





