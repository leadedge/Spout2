{
	"patcher" : 	{
		"fileversion" : 1,
		"rect" : [ 1024.0, 140.0, 529.0, 630.0 ],
		"bglocked" : 0,
		"defrect" : [ 1024.0, 140.0, 529.0, 630.0 ],
		"openrect" : [ 0.0, 0.0, 0.0, 0.0 ],
		"openinpresentation" : 0,
		"default_fontsize" : 12.0,
		"default_fontface" : 0,
		"default_fontname" : "Arial",
		"gridonopen" : 1,
		"gridsize" : [ 15.0, 15.0 ],
		"gridsnaponopen" : 1,
		"toolbarvisible" : 1,
		"boxanimatetime" : 200,
		"imprint" : 0,
		"enablehscroll" : 1,
		"enablevscroll" : 1,
		"devicewidth" : 0.0,
		"boxes" : [ 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "< Plugin dialog",
					"fontsize" : 12.0,
					"patching_rect" : [ 296.0, 487.0, 120.0, 20.0 ],
					"bgcolor" : [ 0.545098, 0.85098, 0.592157, 1.0 ],
					"numinlets" : 1,
					"numoutlets" : 0,
					"fontname" : "Arial",
					"id" : "obj-14"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "< Use sender aspect ratio",
					"fontsize" : 12.0,
					"patching_rect" : [ 265.0, 451.0, 151.0, 20.0 ],
					"bgcolor" : [ 0.545098, 0.85098, 0.592157, 1.0 ],
					"numinlets" : 1,
					"numoutlets" : 0,
					"fontname" : "Arial",
					"id" : "obj-13"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Start a Spout sender. Select a sender using the sender panel. ",
					"linecount" : 3,
					"fontsize" : 14.0,
					"patching_rect" : [ 245.0, 121.0, 153.0, 55.0 ],
					"bgcolor" : [ 0.678431, 0.819608, 0.819608, 1.0 ],
					"numinlets" : 1,
					"numoutlets" : 0,
					"fontname" : "Arial",
					"id" : "obj-9"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "name rwindow",
					"fontsize" : 12.0,
					"patching_rect" : [ 139.0, 194.0, 87.0, 18.0 ],
					"numinlets" : 2,
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"id" : "obj-5"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "param aspect",
					"fontsize" : 12.0,
					"patching_rect" : [ 172.0, 452.0, 83.0, 18.0 ],
					"numinlets" : 2,
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"id" : "obj-4"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "param select",
					"fontsize" : 12.0,
					"patching_rect" : [ 156.0, 418.0, 79.0, 18.0 ],
					"numinlets" : 2,
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"id" : "obj-3"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "jit.pwindow",
					"patching_rect" : [ 138.5, 226.0, 280.0, 165.0 ],
					"numinlets" : 1,
					"numoutlets" : 2,
					"outlettype" : [ "", "" ],
					"id" : "obj-1",
					"name" : "rwindow"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "< Select sender panel",
					"fontface" : 1,
					"fontsize" : 14.0,
					"patching_rect" : [ 247.0, 415.0, 162.0, 23.0 ],
					"bgcolor" : [ 0.545098, 0.85098, 0.592157, 1.0 ],
					"numinlets" : 1,
					"numoutlets" : 0,
					"fontname" : "Arial",
					"id" : "obj-15"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "paramdialog",
					"fontsize" : 12.0,
					"patching_rect" : [ 194.0, 488.0, 93.0, 18.0 ],
					"numinlets" : 2,
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"id" : "obj-11"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "jit.gl.freeframe rwindow @loadeffect SpoutReceiver2",
					"fontsize" : 12.0,
					"patching_rect" : [ 123.0, 529.5, 289.0, 20.0 ],
					"numinlets" : 1,
					"color" : [ 0.239216, 0.643137, 0.709804, 1.0 ],
					"numoutlets" : 2,
					"fontname" : "Arial",
					"outlettype" : [ "", "" ],
					"id" : "obj-33"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "jit.gl.videoplane rwindow @transform_reset 2 @enable 1",
					"fontsize" : 12.0,
					"patching_rect" : [ 123.0, 586.5, 339.0, 20.0 ],
					"numinlets" : 1,
					"numoutlets" : 0,
					"fontname" : "Arial",
					"id" : "obj-34"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "print",
					"fontsize" : 12.0,
					"patching_rect" : [ 393.0, 554.5, 34.0, 20.0 ],
					"numinlets" : 1,
					"numoutlets" : 0,
					"fontname" : "Arial",
					"id" : "obj-44"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "loadbang",
					"fontsize" : 12.0,
					"patching_rect" : [ 44.0, 60.0, 60.0, 20.0 ],
					"numinlets" : 1,
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "bang" ],
					"id" : "obj-26"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Needs a bang to output",
					"linecount" : 3,
					"fontsize" : 12.0,
					"patching_rect" : [ 63.0, 433.0, 58.0, 48.0 ],
					"bgcolor" : [ 0.678431, 0.819608, 0.819608, 1.0 ],
					"numinlets" : 1,
					"numoutlets" : 0,
					"fontname" : "Arial",
					"id" : "obj-7"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "jit.gl.freeframe",
					"fontface" : 2,
					"fontsize" : 14.0,
					"patching_rect" : [ 212.0, 54.0, 108.0, 23.0 ],
					"numinlets" : 1,
					"numoutlets" : 0,
					"fontname" : "Arial",
					"id" : "obj-22"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Spout receiver",
					"fontface" : 1,
					"fontsize" : 24.0,
					"patching_rect" : [ 181.0, 16.0, 181.0, 34.0 ],
					"bgcolor" : [ 0.827451, 0.827451, 0.827451, 1.0 ],
					"numinlets" : 1,
					"numoutlets" : 0,
					"fontname" : "Arial",
					"id" : "obj-21"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "t b b erase",
					"fontsize" : 11.595187,
					"patching_rect" : [ 97.0, 122.0, 65.0, 20.0 ],
					"numinlets" : 1,
					"numoutlets" : 3,
					"fontname" : "Arial",
					"outlettype" : [ "bang", "bang", "erase" ],
					"id" : "obj-69"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "qmetro 30",
					"fontsize" : 11.595187,
					"patching_rect" : [ 97.0, 96.0, 63.0, 20.0 ],
					"numinlets" : 2,
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "bang" ],
					"id" : "obj-71"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "jit.gl.render rwindow",
					"fontsize" : 11.595187,
					"patching_rect" : [ 97.0, 154.0, 113.0, 20.0 ],
					"numinlets" : 1,
					"numoutlets" : 0,
					"fontname" : "Arial",
					"id" : "obj-72"
				}

			}
 ],
		"lines" : [ 			{
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
					"destination" : [ "obj-5", 0 ],
					"hidden" : 0,
					"midpoints" : [ 53.5, 180.5, 148.5, 180.5 ]
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
					"source" : [ "obj-3", 0 ],
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
					"midpoints" : [ 132.5, 545.0, 132.5, 545.0 ]
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
					"source" : [ "obj-4", 0 ],
					"destination" : [ "obj-33", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-5", 0 ],
					"destination" : [ "obj-1", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
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
					"source" : [ "obj-69", 2 ],
					"destination" : [ "obj-72", 0 ],
					"hidden" : 0,
					"midpoints" : [ 152.5, 147.5, 106.5, 147.5 ]
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
					"source" : [ "obj-71", 0 ],
					"destination" : [ "obj-69", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-26", 0 ],
					"destination" : [ "obj-33", 0 ],
					"hidden" : 0,
					"midpoints" : [ 53.5, 494.25, 132.5, 494.25 ]
				}

			}
 ]
	}

}
