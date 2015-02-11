{
	"patcher" : 	{
		"fileversion" : 1,
		"rect" : [ 457.0, 133.0, 585.0, 568.0 ],
		"bglocked" : 0,
		"defrect" : [ 457.0, 133.0, 585.0, 568.0 ],
		"openrect" : [ 0.0, 0.0, 0.0, 0.0 ],
		"openinpresentation" : 0,
		"default_fontsize" : 12.0,
		"default_fontface" : 0,
		"default_fontname" : "Arial",
		"gridonopen" : 0,
		"gridsize" : [ 15.0, 15.0 ],
		"gridsnaponopen" : 0,
		"toolbarvisible" : 1,
		"boxanimatetime" : 200,
		"imprint" : 0,
		"enablehscroll" : 1,
		"enablevscroll" : 1,
		"devicewidth" : 0.0,
		"boxes" : [ 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "< Update name entry",
					"numinlets" : 1,
					"bgcolor" : [ 0.545098, 0.85098, 0.592157, 1.0 ],
					"fontname" : "Arial",
					"numoutlets" : 0,
					"id" : "obj-12",
					"fontsize" : 12.0,
					"patching_rect" : [ 362.0, 380.0, 131.0, 20.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "param update",
					"numinlets" : 2,
					"fontname" : "Arial",
					"numoutlets" : 1,
					"id" : "obj-6",
					"outlettype" : [ "" ],
					"fontsize" : 12.0,
					"patching_rect" : [ 262.0, 380.0, 85.0, 18.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Start a Spout receiver. Output will be visible as a sender named \"maxpatch\". The sender name can be changed with the plugin name entry dialog or manually.",
					"linecount" : 7,
					"numinlets" : 1,
					"bgcolor" : [ 0.678431, 0.819608, 0.819608, 1.0 ],
					"fontname" : "Arial",
					"numoutlets" : 0,
					"id" : "obj-4",
					"fontsize" : 14.0,
					"patching_rect" : [ 275.0, 92.0, 180.0, 119.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "jji.gl.freeframe",
					"numinlets" : 1,
					"fontname" : "Arial",
					"numoutlets" : 0,
					"id" : "obj-22",
					"fontface" : 2,
					"fontsize" : 16.0,
					"patching_rect" : [ 210.0, 51.0, 117.0, 25.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Spout sender",
					"numinlets" : 1,
					"bgcolor" : [ 0.827451, 0.827451, 0.827451, 1.0 ],
					"fontname" : "Arial",
					"numoutlets" : 0,
					"id" : "obj-21",
					"fontface" : 1,
					"fontsize" : 24.0,
					"patching_rect" : [ 188.0, 12.0, 181.0, 34.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Enter sender name",
					"numinlets" : 1,
					"bgcolor" : [ 0.545098, 0.85098, 0.592157, 1.0 ],
					"fontname" : "Arial",
					"numoutlets" : 0,
					"id" : "obj-24",
					"fontface" : 1,
					"fontsize" : 14.0,
					"patching_rect" : [ 370.0, 262.0, 150.0, 23.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "route text",
					"numinlets" : 1,
					"fontname" : "Arial",
					"numoutlets" : 2,
					"id" : "obj-16",
					"outlettype" : [ "", "" ],
					"fontsize" : 12.0,
					"patching_rect" : [ 370.0, 320.0, 61.0, 20.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "textedit",
					"text" : "test",
					"numinlets" : 1,
					"bgcolor" : [ 0.047059, 0.913725, 0.913725, 1.0 ],
					"fontname" : "Arial",
					"numoutlets" : 4,
					"id" : "obj-9",
					"outlettype" : [ "", "int", "", "" ],
					"fontsize" : 12.0,
					"lines" : 1,
					"keymode" : 1,
					"patching_rect" : [ 370.0, 293.0, 102.0, 21.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "param 0 $1",
					"numinlets" : 2,
					"fontname" : "Arial",
					"numoutlets" : 1,
					"id" : "obj-8",
					"outlettype" : [ "" ],
					"fontsize" : 12.0,
					"patching_rect" : [ 370.0, 346.0, 71.0, 18.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "jit.window @name swindow @size 320 240 @floating 1",
					"numinlets" : 0,
					"fontname" : "Arial",
					"numoutlets" : 0,
					"id" : "obj-1",
					"fontsize" : 12.0,
					"patching_rect" : [ 140.5, 224.0, 305.0, 20.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "< Sender name dialog",
					"numinlets" : 1,
					"bgcolor" : [ 0.545098, 0.85098, 0.592157, 1.0 ],
					"fontname" : "Arial",
					"numoutlets" : 0,
					"id" : "obj-15",
					"fontface" : 1,
					"fontsize" : 14.0,
					"patching_rect" : [ 238.0, 423.0, 154.0, 23.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "paramdialog",
					"numinlets" : 2,
					"bgcolor" : [ 0.047059, 0.913725, 0.913725, 1.0 ],
					"fontname" : "Arial",
					"numoutlets" : 1,
					"id" : "obj-11",
					"outlettype" : [ "" ],
					"fontsize" : 12.0,
					"patching_rect" : [ 138.0, 427.0, 93.0, 18.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "jit.gl.freeframe swindow @loadeffect SpoutSender2 @param 0 maxpatch",
					"numinlets" : 1,
					"fontname" : "Arial",
					"numoutlets" : 2,
					"color" : [ 0.239216, 0.643137, 0.709804, 1.0 ],
					"id" : "obj-33",
					"outlettype" : [ "", "" ],
					"fontsize" : 12.0,
					"patching_rect" : [ 120.0, 464.5, 396.0, 20.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "jit.gl.videoplane swindow @transform_reset 2 @enable 1",
					"numinlets" : 1,
					"fontname" : "Arial",
					"numoutlets" : 0,
					"id" : "obj-34",
					"fontsize" : 12.0,
					"patching_rect" : [ 120.0, 497.5, 339.0, 20.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "print",
					"numinlets" : 1,
					"fontname" : "Arial",
					"numoutlets" : 0,
					"id" : "obj-44",
					"fontsize" : 12.0,
					"patching_rect" : [ 497.0, 499.5, 34.0, 20.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "loadbang",
					"numinlets" : 1,
					"fontname" : "Arial",
					"numoutlets" : 1,
					"id" : "obj-26",
					"outlettype" : [ "bang" ],
					"fontsize" : 12.0,
					"patching_rect" : [ 44.0, 96.0, 60.0, 20.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Needs a bang to output",
					"linecount" : 3,
					"numinlets" : 1,
					"bgcolor" : [ 0.678431, 0.819608, 0.819608, 1.0 ],
					"fontname" : "Arial",
					"numoutlets" : 0,
					"id" : "obj-7",
					"fontsize" : 12.0,
					"patching_rect" : [ 56.0, 450.0, 58.0, 48.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "read dozer.mov",
					"numinlets" : 2,
					"bgcolor" : [ 0.678431, 0.819608, 0.819608, 1.0 ],
					"fontname" : "Arial",
					"numoutlets" : 1,
					"id" : "obj-2",
					"outlettype" : [ "" ],
					"fontface" : 1,
					"fontsize" : 16.0,
					"patching_rect" : [ 151.0, 261.0, 128.0, 23.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "jit.gl.texture swindow @name tex",
					"numinlets" : 1,
					"fontname" : "Arial",
					"numoutlets" : 1,
					"id" : "obj-13",
					"outlettype" : [ "" ],
					"fontsize" : 12.0,
					"patching_rect" : [ 151.0, 336.0, 187.0, 20.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "t b b erase",
					"numinlets" : 1,
					"fontname" : "Arial",
					"numoutlets" : 3,
					"id" : "obj-69",
					"outlettype" : [ "bang", "bang", "erase" ],
					"fontsize" : 11.595187,
					"patching_rect" : [ 97.0, 158.0, 65.0, 20.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "qmetro 30",
					"numinlets" : 2,
					"fontname" : "Arial",
					"numoutlets" : 1,
					"id" : "obj-71",
					"outlettype" : [ "bang" ],
					"fontsize" : 11.595187,
					"patching_rect" : [ 97.0, 132.0, 63.0, 20.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "jit.gl.render swindow",
					"numinlets" : 1,
					"fontname" : "Arial",
					"numoutlets" : 0,
					"id" : "obj-72",
					"fontsize" : 11.595187,
					"patching_rect" : [ 97.0, 190.0, 114.0, 20.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "jit.qt.movie @adapt 1",
					"numinlets" : 1,
					"fontname" : "Arial",
					"numoutlets" : 2,
					"id" : "obj-5",
					"outlettype" : [ "jit_matrix", "" ],
					"fontsize" : 12.0,
					"patching_rect" : [ 151.0, 301.0, 121.0, 20.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "qmetro 30",
					"numinlets" : 2,
					"fontname" : "Arial",
					"numoutlets" : 1,
					"id" : "obj-3",
					"outlettype" : [ "bang" ],
					"fontsize" : 12.0,
					"patching_rect" : [ 64.0, 226.0, 65.0, 20.0 ]
				}

			}
 ],
		"lines" : [ 			{
				"patchline" : 				{
					"source" : [ "obj-26", 0 ],
					"destination" : [ "obj-2", 0 ],
					"hidden" : 0,
					"midpoints" : [ 53.5, 254.5, 160.5, 254.5 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-6", 0 ],
					"destination" : [ "obj-33", 0 ],
					"hidden" : 0,
					"midpoints" : [ 271.5, 406.25, 129.5, 406.25 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-8", 0 ],
					"destination" : [ "obj-33", 0 ],
					"hidden" : 0,
					"midpoints" : [ 379.5, 369.75, 129.5, 369.75 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-16", 0 ],
					"destination" : [ "obj-8", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-9", 0 ],
					"destination" : [ "obj-16", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-26", 0 ],
					"destination" : [ "obj-33", 0 ],
					"hidden" : 0,
					"midpoints" : [ 53.5, 439.25, 129.5, 439.25 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-13", 0 ],
					"destination" : [ "obj-33", 0 ],
					"hidden" : 0,
					"midpoints" : [ 160.5, 406.25, 129.5, 406.25 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-69", 1 ],
					"destination" : [ "obj-33", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-33", 0 ],
					"destination" : [ "obj-34", 0 ],
					"hidden" : 0,
					"midpoints" : [ 129.5, 480.0, 129.5, 480.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-33", 1 ],
					"destination" : [ "obj-44", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-11", 0 ],
					"destination" : [ "obj-33", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-26", 0 ],
					"destination" : [ "obj-3", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-26", 0 ],
					"destination" : [ "obj-71", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-2", 0 ],
					"destination" : [ "obj-5", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-69", 0 ],
					"destination" : [ "obj-72", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-69", 2 ],
					"destination" : [ "obj-72", 0 ],
					"hidden" : 0,
					"midpoints" : [ 152.5, 183.5, 106.5, 183.5 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-71", 0 ],
					"destination" : [ "obj-69", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-3", 0 ],
					"destination" : [ "obj-5", 0 ],
					"hidden" : 0,
					"midpoints" : [ 73.5, 290.5, 160.5, 290.5 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-5", 0 ],
					"destination" : [ "obj-13", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
 ]
	}

}
