﻿NDSummary.OnToolTipsLoaded("CClass:Spout",{2:"<div class=\"NDToolTip TClass LC\"><div class=\"NDClassPrototype\" id=\"NDClassPrototype2\"><div class=\"CPEntry TClass Current\"><div class=\"CPName\">Spout&#8203;<span class=\"TemplateSignature\">()</span></div></div></div><div class=\"TTSummary\">https://&#8203;spout&#8203;.zeal&#8203;.co&#8203;/</div></div>",3:"<div class=\"NDToolTip TGroup LC\"><div class=\"TTSummary\">SendFbo, SendTexture and SendImage create or update a sender as required.</div></div>",4:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype4\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">void</span> Spout::SetSenderName(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PModifierQualifier first\"><span class=\"SHKeyword\">const</span>&nbsp;</td><td class=\"PType\"><span class=\"SHKeyword\">char</span>*&nbsp;</td><td class=\"PName last\">sendername</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Set name for sender creation</div></div>",5:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype5\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">void</span> Spout::SetSenderFormat(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\">DWORD&nbsp;</td><td class=\"PName last\">dwFormat</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Set the sender DX11 shared texture format</div></div>",6:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype6\" class=\"NDPrototype\"><div class=\"PSection PPlainSection\"><span class=\"SHKeyword\">void</span> Spout::ReleaseSender()</div></div><div class=\"TTSummary\">Close receiver and release resources.</div></div>",7:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype7\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">bool</span> Spout::SendFbo(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"first\"></td><td class=\"PType\">GLuint&nbsp;</td><td class=\"PName last\">FboID,</td></tr><tr><td class=\"PModifierQualifier first\"><span class=\"SHKeyword\">unsigned</span>&nbsp;</td><td class=\"PType\"><span class=\"SHKeyword\">int</span>&nbsp;</td><td class=\"PName last\">width,</td></tr><tr><td class=\"PModifierQualifier first\"><span class=\"SHKeyword\">unsigned</span>&nbsp;</td><td class=\"PType\"><span class=\"SHKeyword\">int</span>&nbsp;</td><td class=\"PName last\">height,</td></tr><tr><td class=\"first\"></td><td class=\"PType\"><span class=\"SHKeyword\">bool</span>&nbsp;</td><td class=\"PName last\">bInvert</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Send texture attached to fbo.</div></div>",8:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype8\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">bool</span> Spout::SendTexture(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"first\"></td><td class=\"PType\">GLuint&nbsp;</td><td class=\"PName last\">TextureID,</td></tr><tr><td class=\"first\"></td><td class=\"PType\">GLuint&nbsp;</td><td class=\"PName last\">TextureTarget,</td></tr><tr><td class=\"PModifierQualifier first\"><span class=\"SHKeyword\">unsigned</span>&nbsp;</td><td class=\"PType\"><span class=\"SHKeyword\">int</span>&nbsp;</td><td class=\"PName last\">width,</td></tr><tr><td class=\"PModifierQualifier first\"><span class=\"SHKeyword\">unsigned</span>&nbsp;</td><td class=\"PType\"><span class=\"SHKeyword\">int</span>&nbsp;</td><td class=\"PName last\">height,</td></tr><tr><td class=\"first\"></td><td class=\"PType\"><span class=\"SHKeyword\">bool</span>&nbsp;</td><td class=\"PName last\">bInvert,</td></tr><tr><td class=\"first\"></td><td class=\"PType\">GLuint&nbsp;</td><td class=\"PName last\">HostFBO</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Send OpenGL texture</div></div>",9:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype9\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">bool</span> Spout::SendImage(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PModifierQualifier first\"><span class=\"SHKeyword\">const unsigned</span>&nbsp;</td><td class=\"PType\"><span class=\"SHKeyword\">char</span>*&nbsp;</td><td class=\"PName last\">pixels,</td></tr><tr><td class=\"PModifierQualifier first\"><span class=\"SHKeyword\">unsigned</span>&nbsp;</td><td class=\"PType\"><span class=\"SHKeyword\">int</span>&nbsp;</td><td class=\"PName last\">width,</td></tr><tr><td class=\"PModifierQualifier first\"><span class=\"SHKeyword\">unsigned</span>&nbsp;</td><td class=\"PType\"><span class=\"SHKeyword\">int</span>&nbsp;</td><td class=\"PName last\">height,</td></tr><tr><td class=\"first\"></td><td class=\"PType\">GLenum&nbsp;</td><td class=\"PName last\">glFormat,</td></tr><tr><td class=\"first\"></td><td class=\"PType\"><span class=\"SHKeyword\">bool</span>&nbsp;</td><td class=\"PName last\">bInvert,</td></tr><tr><td class=\"first\"></td><td class=\"PType\">GLuint&nbsp;</td><td class=\"PName last\">HostFBO</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Send pixel image</div></div>",10:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype10\" class=\"NDPrototype\"><div class=\"PSection PPlainSection\"><span class=\"SHKeyword\">bool</span> Spout::IsInitialized()</div></div><div class=\"TTSummary\">Initialization status</div></div>",11:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype11\" class=\"NDPrototype\"><div class=\"PSection PPlainSection\"><span class=\"SHKeyword\">const char</span> * Spout::GetName()</div></div><div class=\"TTSummary\">Sender name</div></div>",12:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype12\" class=\"NDPrototype\"><div class=\"PSection PPlainSection\"><span class=\"SHKeyword\">unsigned int</span> Spout::GetWidth()</div></div><div class=\"TTSummary\">Sender width</div></div>",13:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype13\" class=\"NDPrototype\"><div class=\"PSection PPlainSection\"><span class=\"SHKeyword\">unsigned int</span> Spout::GetHeight()</div></div><div class=\"TTSummary\">Sender height</div></div>",14:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype14\" class=\"NDPrototype\"><div class=\"PSection PPlainSection\"><span class=\"SHKeyword\">double</span> Spout::GetFps()</div></div><div class=\"TTSummary\">Sender frame rate</div></div>",15:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype15\" class=\"NDPrototype\"><div class=\"PSection PPlainSection\"><span class=\"SHKeyword\">long</span> Spout::GetFrame()</div></div><div class=\"TTSummary\">Sender frame number</div></div>",16:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype16\" class=\"NDPrototype\"><div class=\"PSection PPlainSection\">HANDLE Spout::GetHandle()</div></div><div class=\"TTSummary\">Sender share handle</div></div>",17:"<div class=\"NDToolTip TGroup LC\"><div class=\"TTSummary\">ReceiveTexture and ReceiveImage</div></div>",22:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype22\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">void</span> Spout::SetReceiverName(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PModifierQualifier first\"><span class=\"SHKeyword\">const</span>&nbsp;</td><td class=\"PType\"><span class=\"SHKeyword\">char</span> *&nbsp;</td><td class=\"PName last\">SenderName</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Specify sender for connection</div></div>",23:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype23\" class=\"NDPrototype\"><div class=\"PSection PPlainSection\"><span class=\"SHKeyword\">void</span> Spout::ReleaseReceiver()</div></div><div class=\"TTSummary\">Close receiver and release resources ready to connect to another sender</div></div>",24:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype24\" class=\"NDPrototype\"><div class=\"PSection PPlainSection\"><span class=\"SHKeyword\">bool</span> Spout::ReceiveTexture()</div></div><div class=\"TTSummary\">Connect to a sender and retrieve shared texture details</div></div>",25:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype25\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">bool</span> Spout::ReceiveTexture(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\">GLuint&nbsp;</td><td class=\"PName last\">TextureID,</td></tr><tr><td class=\"PType first\">GLuint&nbsp;</td><td class=\"PName last\">TextureTarget,</td></tr><tr><td class=\"PType first\"><span class=\"SHKeyword\">bool</span>&nbsp;</td><td class=\"PName last\">bInvert,</td></tr><tr><td class=\"PType first\">GLuint&nbsp;</td><td class=\"PName last\">HostFbo</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Copy the sender shared texture</div></div>",26:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype26\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">bool</span> Spout::ReceiveImage(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"first\"></td><td class=\"PType\"><span class=\"SHKeyword\">char</span>*&nbsp;</td><td></td><td class=\"PName last\">Sendername,</td></tr><tr><td class=\"PModifierQualifier first\"><span class=\"SHKeyword\">unsigned</span>&nbsp;</td><td class=\"PType\"><span class=\"SHKeyword\">int</span>&nbsp;</td><td class=\"PSymbols\">&amp;</td><td class=\"PName last\">width,</td></tr><tr><td class=\"PModifierQualifier first\"><span class=\"SHKeyword\">unsigned</span>&nbsp;</td><td class=\"PType\"><span class=\"SHKeyword\">int</span>&nbsp;</td><td class=\"PSymbols\">&amp;</td><td class=\"PName last\">height,</td></tr><tr><td class=\"PModifierQualifier first\"><span class=\"SHKeyword\">unsigned</span>&nbsp;</td><td class=\"PType\"><span class=\"SHKeyword\">char</span>*&nbsp;</td><td></td><td class=\"PName last\">pixels,</td></tr><tr><td class=\"first\"></td><td class=\"PType\">GLenum&nbsp;</td><td></td><td class=\"PName last\">glFormat,</td></tr><tr><td class=\"first\"></td><td class=\"PType\"><span class=\"SHKeyword\">bool</span>&nbsp;</td><td></td><td class=\"PName last\">bInvert,</td></tr><tr><td class=\"first\"></td><td class=\"PType\">GLuint&nbsp;</td><td></td><td class=\"PName last\">HostFBO</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Copy the sender texture to image pixels.</div></div>",27:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype27\" class=\"NDPrototype\"><div class=\"PSection PPlainSection\"><span class=\"SHKeyword\">bool</span> Spout::IsUpdated()</div></div><div class=\"TTSummary\">Query whether the sender has changed.</div></div>",28:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype28\" class=\"NDPrototype\"><div class=\"PSection PPlainSection\"><span class=\"SHKeyword\">bool</span> Spout::IsConnected()</div></div><div class=\"TTSummary\">Query sender connection.</div></div>",29:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype29\" class=\"NDPrototype\"><div class=\"PSection PPlainSection\"><span class=\"SHKeyword\">bool</span> Spout::IsFrameNew()</div></div><div class=\"TTSummary\">Query received frame status</div></div>",30:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype30\" class=\"NDPrototype\"><div class=\"PSection PPlainSection\">DWORD Spout::GetSenderFormat()</div></div><div class=\"TTSummary\">Get sender DirectX texture format</div></div>",31:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype31\" class=\"NDPrototype\"><div class=\"PSection PPlainSection\"><span class=\"SHKeyword\">const char</span> * Spout::GetSenderName()</div></div><div class=\"TTSummary\">Get sender name</div></div>",32:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype32\" class=\"NDPrototype\"><div class=\"PSection PPlainSection\"><span class=\"SHKeyword\">unsigned int</span> Spout::GetSenderWidth()</div></div><div class=\"TTSummary\">Get sender width</div></div>",33:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype33\" class=\"NDPrototype\"><div class=\"PSection PPlainSection\"><span class=\"SHKeyword\">unsigned int</span> Spout::GetSenderHeight()</div></div><div class=\"TTSummary\">Get sender height</div></div>",34:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype34\" class=\"NDPrototype\"><div class=\"PSection PPlainSection\"><span class=\"SHKeyword\">double</span> Spout::GetSenderFps()</div></div><div class=\"TTSummary\">Get sender frame rate</div></div>",35:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype35\" class=\"NDPrototype\"><div class=\"PSection PPlainSection\"><span class=\"SHKeyword\">long</span> Spout::GetSenderFrame()</div></div><div class=\"TTSummary\">Get sender frame number</div></div>",36:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype36\" class=\"NDPrototype\"><div class=\"PSection PPlainSection\">HANDLE Spout::GetSenderHandle()</div></div><div class=\"TTSummary\">Received sender share handle</div></div>",37:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype37\" class=\"NDPrototype\"><div class=\"PSection PPlainSection\"><span class=\"SHKeyword\">bool</span> Spout::GetSenderCPUmode()</div></div><div class=\"TTSummary\">Received sender sharing mode</div></div>",38:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype38\" class=\"NDPrototype\"><div class=\"PSection PPlainSection\"><span class=\"SHKeyword\">void</span> Spout::SelectSender()</div></div><div class=\"TTSummary\">Open sender selection dialog</div></div>",40:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype40\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">void</span> Spout::SetFrameCount(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\"><span class=\"SHKeyword\">bool</span>&nbsp;</td><td class=\"PName last\">bEnable</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Enable or disable frame counting globally</div></div>",41:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype41\" class=\"NDPrototype\"><div class=\"PSection PPlainSection\"><span class=\"SHKeyword\">void</span> Spout::DisableFrameCount()</div></div><div class=\"TTSummary\">Disable frame counting specifically for this application</div></div>",42:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype42\" class=\"NDPrototype\"><div class=\"PSection PPlainSection\"><span class=\"SHKeyword\">bool</span> Spout::IsFrameCountEnabled()</div></div><div class=\"TTSummary\">Return frame count status</div></div>",43:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype43\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">void</span> Spout::HoldFps(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\"><span class=\"SHKeyword\">int</span>&nbsp;</td><td class=\"PName last\">fps</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Frame rate control</div></div>",45:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype45\" class=\"NDPrototype\"><div class=\"PSection PPlainSection\"><span class=\"SHKeyword\">int</span> Spout::GetSenderCount()</div></div><div class=\"TTSummary\">Number of senders</div></div>",46:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype46\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">bool</span> Spout::GetSender(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\"><span class=\"SHKeyword\">int</span>&nbsp;</td><td class=\"PName last\">index,</td></tr><tr><td class=\"PType first\"><span class=\"SHKeyword\">char</span>*&nbsp;</td><td class=\"PName last\">sendername,</td></tr><tr><td class=\"PType first\"><span class=\"SHKeyword\">int</span>&nbsp;</td><td class=\"PName last\">sendernameMaxSize</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Sender item name in the sender names set</div></div>",47:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype47\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">bool</span> Spout::GetSenderInfo(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PModifierQualifier first\"><span class=\"SHKeyword\">const</span>&nbsp;</td><td class=\"PType\"><span class=\"SHKeyword\">char</span>*&nbsp;</td><td></td><td class=\"PName last\">sendername,</td></tr><tr><td class=\"PModifierQualifier first\"><span class=\"SHKeyword\">unsigned</span>&nbsp;</td><td class=\"PType\"><span class=\"SHKeyword\">int</span>&nbsp;</td><td class=\"PSymbols\">&amp;</td><td class=\"PName last\">width,</td></tr><tr><td class=\"PModifierQualifier first\"><span class=\"SHKeyword\">unsigned</span>&nbsp;</td><td class=\"PType\"><span class=\"SHKeyword\">int</span>&nbsp;</td><td class=\"PSymbols\">&amp;</td><td class=\"PName last\">height,</td></tr><tr><td class=\"first\"></td><td class=\"PType\">HANDLE&nbsp;</td><td class=\"PSymbols\">&amp;</td><td class=\"PName last\">dxShareHandle,</td></tr><tr><td class=\"first\"></td><td class=\"PType\">DWORD&nbsp;</td><td class=\"PSymbols\">&amp;</td><td class=\"PName last\">dwFormat</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Sender information</div></div>",48:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype48\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">bool</span> Spout::GetActiveSender(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\"><span class=\"SHKeyword\">char</span>*&nbsp;</td><td class=\"PName last\">Sendername</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Current active sender</div></div>",50:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype50\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">bool</span> Spout::SetActiveSender(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PModifierQualifier first\"><span class=\"SHKeyword\">const</span>&nbsp;</td><td class=\"PType\"><span class=\"SHKeyword\">char</span>*&nbsp;</td><td class=\"PName last\">Sendername</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Set sender as active</div></div>",52:"<div class=\"NDToolTip TGroup LC\"><div class=\"TTSummary\">Note that both the Sender and Receiver must use the same graphics adapter.</div></div>",58:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype58\" class=\"NDPrototype\"><div class=\"PSection PPlainSection\"><span class=\"SHKeyword\">int</span> Spout::GetNumAdapters()</div></div><div class=\"TTSummary\">The number of graphics adapters in the system</div></div>",59:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype59\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">bool</span> Spout::GetAdapterName(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\"><span class=\"SHKeyword\">int</span>&nbsp;</td><td></td><td class=\"PName last\">index,</td></tr><tr><td class=\"PType first\"><span class=\"SHKeyword\">char</span>&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">adaptername,</td></tr><tr><td class=\"PType first\"><span class=\"SHKeyword\">int</span>&nbsp;</td><td></td><td class=\"PName last\">maxchars</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Get adapter item name</div></div>",60:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype60\" class=\"NDPrototype\"><div class=\"PSection PPlainSection\"><span class=\"SHKeyword\">int</span> Spout::GetAdapter()</div></div><div class=\"TTSummary\">Get adapter index</div></div>",61:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype61\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">bool</span> Spout::SetAdapter(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\"><span class=\"SHKeyword\">int</span>&nbsp;</td><td class=\"PName last\">index</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Set graphics adapter for output</div></div>",62:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype62\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">bool</span> Spout::GetAdapterInfo(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\"><span class=\"SHKeyword\">char</span>&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">renderdescription,</td></tr><tr><td class=\"PType first\"><span class=\"SHKeyword\">char</span>&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">displaydescription,</td></tr><tr><td class=\"PType first\"><span class=\"SHKeyword\">int</span>&nbsp;</td><td></td><td class=\"PName last\">maxchars</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Get the current adapter description</div></div>",63:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype63\" class=\"NDPrototype\"><div class=\"PSection PPlainSection\"><span class=\"SHKeyword\">int</span> Spout::Adapter()</div></div><div class=\"TTSummary\">Current adapter</div></div>",64:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype64\" class=\"NDPrototype\"><div class=\"PSection PPlainSection\"><span class=\"SHKeyword\">char</span> * Spout::AdapterName()</div></div><div class=\"TTSummary\">Current adapter name</div></div>",65:"<div class=\"NDToolTip TGroup LC\"><div class=\"TTSummary\">These functions are not necessary for Version 2.007.&nbsp; But are retained for compatibility with existing 2.006 code.</div></div>",66:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype66\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">bool</span> Spout::FindNVIDIA(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\"><span class=\"SHKeyword\">int</span>&nbsp;</td><td class=\"PSymbols\">&amp;</td><td class=\"PName last\">nAdapter</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Find the index of the NVIDIA adapter in a multi-adapter system</div></div>",67:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype67\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">bool</span> Spout::GetAdapterInfo(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\"><span class=\"SHKeyword\">char</span>*&nbsp;</td><td></td><td class=\"PName last\">renderadapter,</td></tr><tr><td class=\"PType first\"><span class=\"SHKeyword\">char</span>*&nbsp;</td><td></td><td class=\"PName last\">renderdescription,</td></tr><tr><td class=\"PType first\"><span class=\"SHKeyword\">char</span>*&nbsp;</td><td></td><td class=\"PName last\">renderversion,</td></tr><tr><td class=\"PType first\"><span class=\"SHKeyword\">char</span>*&nbsp;</td><td></td><td class=\"PName last\">displaydescription,</td></tr><tr><td class=\"PType first\"><span class=\"SHKeyword\">char</span>*&nbsp;</td><td></td><td class=\"PName last\">displayversion,</td></tr><tr><td class=\"PType first\"><span class=\"SHKeyword\">int</span>&nbsp;</td><td></td><td class=\"PName last\">maxsize,</td></tr><tr><td class=\"PType first\"><span class=\"SHKeyword\">bool</span>&nbsp;</td><td class=\"PSymbols\">&amp;</td><td class=\"PName last\">bDX9</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Get detailed information for the current graphics adapter</div></div>",68:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype68\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">bool</span> Spout::CreateSender(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PModifierQualifier first\"><span class=\"SHKeyword\">const</span>&nbsp;</td><td class=\"PType\"><span class=\"SHKeyword\">char</span>*&nbsp;</td><td class=\"PName last\">name,</td></tr><tr><td class=\"PModifierQualifier first\"><span class=\"SHKeyword\">unsigned</span>&nbsp;</td><td class=\"PType\"><span class=\"SHKeyword\">int</span>&nbsp;</td><td class=\"PName last\">width,</td></tr><tr><td class=\"PModifierQualifier first\"><span class=\"SHKeyword\">unsigned</span>&nbsp;</td><td class=\"PType\"><span class=\"SHKeyword\">int</span>&nbsp;</td><td class=\"PName last\">height,</td></tr><tr><td class=\"first\"></td><td class=\"PType\">DWORD&nbsp;</td><td class=\"PName last\">dwFormat</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Create a sender</div></div>",69:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype69\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">bool</span> Spout::UpdateSender(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PModifierQualifier first\"><span class=\"SHKeyword\">const</span>&nbsp;</td><td class=\"PType\"><span class=\"SHKeyword\">char</span>*&nbsp;</td><td class=\"PName last\">name,</td></tr><tr><td class=\"PModifierQualifier first\"><span class=\"SHKeyword\">unsigned</span>&nbsp;</td><td class=\"PType\"><span class=\"SHKeyword\">int</span>&nbsp;</td><td class=\"PName last\">width,</td></tr><tr><td class=\"PModifierQualifier first\"><span class=\"SHKeyword\">unsigned</span>&nbsp;</td><td class=\"PType\"><span class=\"SHKeyword\">int</span>&nbsp;</td><td class=\"PName last\">height</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Update a sender</div></div>",70:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype70\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">bool</span> Spout::DrawToSharedTexture(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"first\"></td><td class=\"PType\">GLuint&nbsp;</td><td class=\"PName last\">TextureID,</td></tr><tr><td class=\"first\"></td><td class=\"PType\">GLuint&nbsp;</td><td class=\"PName last\">TextureTarget,</td></tr><tr><td class=\"PModifierQualifier first\"><span class=\"SHKeyword\">unsigned</span>&nbsp;</td><td class=\"PType\"><span class=\"SHKeyword\">int</span>&nbsp;</td><td class=\"PName last\">width,</td></tr><tr><td class=\"PModifierQualifier first\"><span class=\"SHKeyword\">unsigned</span>&nbsp;</td><td class=\"PType\"><span class=\"SHKeyword\">int</span>&nbsp;</td><td class=\"PName last\">height,</td></tr><tr><td class=\"first\"></td><td class=\"PType\"><span class=\"SHKeyword\">float</span>&nbsp;</td><td class=\"PName last\">max_x,</td></tr><tr><td class=\"first\"></td><td class=\"PType\"><span class=\"SHKeyword\">float</span>&nbsp;</td><td class=\"PName last\">max_y,</td></tr><tr><td class=\"first\"></td><td class=\"PType\"><span class=\"SHKeyword\">float</span>&nbsp;</td><td class=\"PName last\">aspect,</td></tr><tr><td class=\"first\"></td><td class=\"PType\"><span class=\"SHKeyword\">bool</span>&nbsp;</td><td class=\"PName last\">bInvert,</td></tr><tr><td class=\"first\"></td><td class=\"PType\">GLuint&nbsp;</td><td class=\"PName last\">HostFBO</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Render OpenGL texture to the sender shared OpenGL texture</div></div>",71:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype71\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">bool</span> Spout::CreateReceiver(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"first\"></td><td class=\"PType\"><span class=\"SHKeyword\">char</span>*&nbsp;</td><td></td><td class=\"PName last\">sendername,</td></tr><tr><td class=\"PModifierQualifier first\"><span class=\"SHKeyword\">unsigned</span>&nbsp;</td><td class=\"PType\"><span class=\"SHKeyword\">int</span>&nbsp;</td><td class=\"PSymbols\">&amp;</td><td class=\"PName last\">width,</td></tr><tr><td class=\"PModifierQualifier first\"><span class=\"SHKeyword\">unsigned</span>&nbsp;</td><td class=\"PType\"><span class=\"SHKeyword\">int</span>&nbsp;</td><td class=\"PSymbols\">&amp;</td><td class=\"PName last\">height,</td></tr><tr><td class=\"first\"></td><td class=\"PType\"><span class=\"SHKeyword\">bool</span>&nbsp;</td><td></td><td class=\"PName last\">bUseActive</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Create receiver connection</div></div>",72:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype72\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">bool</span> Spout::CheckReceiver(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"first\"></td><td class=\"PType\"><span class=\"SHKeyword\">char</span>*&nbsp;</td><td></td><td class=\"PName last\">name,</td></tr><tr><td class=\"PModifierQualifier first\"><span class=\"SHKeyword\">unsigned</span>&nbsp;</td><td class=\"PType\"><span class=\"SHKeyword\">int</span>&nbsp;</td><td class=\"PSymbols\">&amp;</td><td class=\"PName last\">width,</td></tr><tr><td class=\"PModifierQualifier first\"><span class=\"SHKeyword\">unsigned</span>&nbsp;</td><td class=\"PType\"><span class=\"SHKeyword\">int</span>&nbsp;</td><td class=\"PSymbols\">&amp;</td><td class=\"PName last\">height,</td></tr><tr><td class=\"first\"></td><td class=\"PType\"><span class=\"SHKeyword\">bool</span>&nbsp;</td><td class=\"PSymbols\">&amp;</td><td class=\"PName last\">bConnected</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Check receiver connection</div></div>",73:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype73\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">bool</span> Spout::ReceiveTexture(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"first\"></td><td class=\"PType\"><span class=\"SHKeyword\">char</span>*&nbsp;</td><td></td><td class=\"PName last\">name,</td></tr><tr><td class=\"PModifierQualifier first\"><span class=\"SHKeyword\">unsigned</span>&nbsp;</td><td class=\"PType\"><span class=\"SHKeyword\">int</span>&nbsp;</td><td class=\"PSymbols\">&amp;</td><td class=\"PName last\">width,</td></tr><tr><td class=\"PModifierQualifier first\"><span class=\"SHKeyword\">unsigned</span>&nbsp;</td><td class=\"PType\"><span class=\"SHKeyword\">int</span>&nbsp;</td><td class=\"PSymbols\">&amp;</td><td class=\"PName last\">height,</td></tr><tr><td class=\"first\"></td><td class=\"PType\">GLuint&nbsp;</td><td></td><td class=\"PName last\">TextureID,</td></tr><tr><td class=\"first\"></td><td class=\"PType\">GLuint&nbsp;</td><td></td><td class=\"PName last\">TextureTarget,</td></tr><tr><td class=\"first\"></td><td class=\"PType\"><span class=\"SHKeyword\">bool</span>&nbsp;</td><td></td><td class=\"PName last\">bInvert,</td></tr><tr><td class=\"first\"></td><td class=\"PType\">GLuint&nbsp;</td><td></td><td class=\"PName last\">HostFBO</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Receive OpenGL texture</div></div>",74:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype74\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">bool</span> Spout::ReceiveImage(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PModifierQualifier first\"><span class=\"SHKeyword\">unsigned</span>&nbsp;</td><td class=\"PType\"><span class=\"SHKeyword\">char</span>&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">pixels,</td></tr><tr><td class=\"first\"></td><td class=\"PType\">GLenum&nbsp;</td><td></td><td class=\"PName last\">glFormat,</td></tr><tr><td class=\"first\"></td><td class=\"PType\"><span class=\"SHKeyword\">bool</span>&nbsp;</td><td></td><td class=\"PName last\">bInvert,</td></tr><tr><td class=\"first\"></td><td class=\"PType\">GLuint&nbsp;</td><td></td><td class=\"PName last\">HostFbo</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Receive image pixels</div></div>",75:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype75\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">bool</span> Spout::SelectSenderPanel(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PModifierQualifier first\"><span class=\"SHKeyword\">const</span>&nbsp;</td><td class=\"PType\"><span class=\"SHKeyword\">char</span>&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">message</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Open dialog for the user to select a sender</div></div>",76:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype76\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">bool</span> Spout::DrawSharedTexture(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\"><span class=\"SHKeyword\">float</span>&nbsp;</td><td class=\"PName last\">max_x,</td></tr><tr><td class=\"PType first\"><span class=\"SHKeyword\">float</span>&nbsp;</td><td class=\"PName last\">max_y,</td></tr><tr><td class=\"PType first\"><span class=\"SHKeyword\">float</span>&nbsp;</td><td class=\"PName last\">aspect,</td></tr><tr><td class=\"PType first\"><span class=\"SHKeyword\">bool</span>&nbsp;</td><td class=\"PName last\">bInvert,</td></tr><tr><td class=\"PType first\">GLuint&nbsp;</td><td class=\"PName last\">HostFBO</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Render the sender shared OpenGL texture</div></div>"});