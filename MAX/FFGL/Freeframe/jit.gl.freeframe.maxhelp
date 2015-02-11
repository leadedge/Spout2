{
	"patcher" : 	{
		"fileversion" : 1,
		"rect" : [ 587.0, 72.0, 755.0, 914.0 ],
		"bglocked" : 0,
		"defrect" : [ 587.0, 72.0, 755.0, 914.0 ],
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
					"maxclass" : "newobj",
					"text" : "loadbang",
					"fontname" : "Arial",
					"outlettype" : [ "bang" ],
					"id" : "obj-26",
					"fontsize" : 12.0,
					"numinlets" : 1,
					"patching_rect" : [ 67.0, 52.0, 60.0, 20.0 ],
					"numoutlets" : 1
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Select an effect",
					"fontname" : "Arial",
					"id" : "obj-9",
					"fontface" : 1,
					"fontsize" : 14.0,
					"bgcolor" : [ 0.239216, 0.643137, 0.709804, 1.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 412.0, 676.0, 124.0, 23.0 ],
					"numoutlets" : 0
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "t s b",
					"fontname" : "Arial",
					"outlettype" : [ "", "bang" ],
					"id" : "obj-53",
					"fontsize" : 12.0,
					"numinlets" : 1,
					"patching_rect" : [ 93.0, 645.0, 33.0, 20.0 ],
					"numoutlets" : 2
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Print parameter properties",
					"fontname" : "Arial",
					"id" : "obj-40",
					"fontsize" : 12.0,
					"bgcolor" : [ 0.678431, 0.819608, 0.819608, 1.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 362.0, 520.0, 150.0, 20.0 ],
					"numoutlets" : 0
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "getparamlist",
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"id" : "obj-39",
					"fontsize" : 12.0,
					"bgcolor" : [ 0.545098, 0.85098, 0.592157, 1.0 ],
					"numinlets" : 2,
					"patching_rect" : [ 278.5, 520.0, 76.0, 18.0 ],
					"numoutlets" : 1
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Open a parameter dialog",
					"fontname" : "Arial",
					"id" : "obj-38",
					"fontface" : 1,
					"fontsize" : 14.0,
					"bgcolor" : [ 0.239216, 0.643137, 0.709804, 1.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 217.0, 394.5, 185.0, 23.0 ],
					"numoutlets" : 0
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "paramdialog",
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"id" : "obj-20",
					"fontsize" : 12.0,
					"bgcolor" : [ 0.545098, 0.85098, 0.592157, 1.0 ],
					"numinlets" : 2,
					"patching_rect" : [ 119.0, 398.5, 93.0, 18.0 ],
					"numoutlets" : 1
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "print",
					"fontname" : "Arial",
					"id" : "obj-14",
					"fontsize" : 12.0,
					"numinlets" : 1,
					"patching_rect" : [ 387.0, 785.0, 34.0, 20.0 ],
					"numoutlets" : 0
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "loadbang",
					"fontname" : "Arial",
					"outlettype" : [ "bang" ],
					"id" : "obj-13",
					"fontsize" : 12.0,
					"numinlets" : 1,
					"patching_rect" : [ 116.0, 785.0, 60.0, 20.0 ],
					"numoutlets" : 1
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "button",
					"outlettype" : [ "bang" ],
					"id" : "obj-49",
					"numinlets" : 1,
					"patching_rect" : [ 116.0, 823.0, 24.0, 24.0 ],
					"numoutlets" : 1
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "\nloading \neffects\n\n",
					"linecount" : 4,
					"fontname" : "Arial",
					"id" : "obj-47",
					"fontsize" : 16.0,
					"bgcolor" : [ 1.0, 1.0, 1.0, 1.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 449.0, 570.0, 108.0, 80.0 ],
					"numoutlets" : 0
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "\nchanging \nparameters\n\n",
					"linecount" : 4,
					"fontname" : "Arial",
					"id" : "obj-46",
					"fontsize" : 16.0,
					"bgcolor" : [ 1.0, 1.0, 1.0, 1.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 414.0, 376.0, 155.0, 80.0 ],
					"numoutlets" : 0
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "loadbang",
					"fontname" : "Arial",
					"outlettype" : [ "bang" ],
					"id" : "obj-43",
					"fontsize" : 12.0,
					"numinlets" : 1,
					"patching_rect" : [ 93.0, 575.0, 60.0, 20.0 ],
					"numoutlets" : 1
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "geteffectlist",
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"id" : "obj-45",
					"fontsize" : 12.0,
					"numinlets" : 2,
					"patching_rect" : [ 93.0, 610.0, 72.0, 18.0 ],
					"numoutlets" : 1
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "prepend loadeffect",
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"id" : "obj-35",
					"fontsize" : 12.0,
					"numinlets" : 1,
					"patching_rect" : [ 309.5, 706.0, 110.0, 20.0 ],
					"numoutlets" : 1
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "clear",
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"id" : "obj-36",
					"fontsize" : 12.0,
					"numinlets" : 2,
					"patching_rect" : [ 187.0, 637.0, 37.0, 18.0 ],
					"numoutlets" : 1
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "prepend append",
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"id" : "obj-37",
					"fontsize" : 12.0,
					"numinlets" : 1,
					"patching_rect" : [ 233.5, 637.0, 98.0, 20.0 ],
					"numoutlets" : 1
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "route effectlist end",
					"fontname" : "Arial",
					"outlettype" : [ "", "", "" ],
					"id" : "obj-31",
					"fontsize" : 12.0,
					"numinlets" : 1,
					"patching_rect" : [ 233.5, 598.0, 108.0, 20.0 ],
					"numoutlets" : 3
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "umenu",
					"fontname" : "Arial",
					"outlettype" : [ "int", "", "" ],
					"types" : [  ],
					"id" : "obj-32",
					"fontface" : 1,
					"fontsize" : 12.0,
					"numinlets" : 1,
					"patching_rect" : [ 233.5, 675.0, 171.0, 20.0 ],
					"numoutlets" : 3,
					"framecolor" : [ 0.239216, 0.643137, 0.709804, 1.0 ],
					"items" : "<empty>"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "r bang",
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"id" : "obj-30",
					"fontsize" : 12.0,
					"numinlets" : 0,
					"patching_rect" : [ 11.0, 97.0, 45.0, 20.0 ],
					"numoutlets" : 1
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "s bang",
					"fontname" : "Arial",
					"id" : "obj-29",
					"fontsize" : 12.0,
					"numinlets" : 1,
					"patching_rect" : [ 511.0, 212.0, 47.0, 20.0 ],
					"numoutlets" : 0
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "loadbang",
					"fontname" : "Arial",
					"outlettype" : [ "bang" ],
					"id" : "obj-28",
					"fontsize" : 12.0,
					"numinlets" : 1,
					"patching_rect" : [ 282.5, 127.0, 60.0, 20.0 ],
					"numoutlets" : 1
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Effects in :\n\"..\\Program Files\\Common Files\\Freeframe\\\"\nor in same directory as patch",
					"linecount" : 3,
					"fontname" : "Arial",
					"id" : "obj-8",
					"fontsize" : 12.0,
					"bgcolor" : [ 1.0, 1.0, 1.0, 1.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 449.0, 714.0, 248.0, 48.0 ],
					"numoutlets" : 0
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Needs a bang to initialize",
					"fontname" : "Arial",
					"id" : "obj-7",
					"fontface" : 1,
					"fontsize" : 12.0,
					"bgcolor" : [ 0.858824, 0.858824, 0.858824, 1.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 142.0, 826.5, 152.0, 20.0 ],
					"numoutlets" : 0
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Processed through jit.gl.freeframe plugin",
					"linecount" : 2,
					"fontname" : "Arial",
					"id" : "obj-24",
					"fontsize" : 12.0,
					"bgcolor" : [ 0.678431, 0.819608, 0.819608, 1.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 282.5, 312.0, 171.0, 34.0 ],
					"numoutlets" : 0
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Before ..",
					"fontname" : "Arial",
					"id" : "obj-25",
					"fontsize" : 12.0,
					"bgcolor" : [ 0.678431, 0.819608, 0.819608, 1.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 93.0, 312.0, 171.0, 20.0 ],
					"numoutlets" : 0
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Change a parameter Value (0-1)",
					"fontname" : "Arial",
					"id" : "obj-15",
					"fontsize" : 12.0,
					"bgcolor" : [ 0.678431, 0.819608, 0.819608, 1.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 255.0, 466.0, 184.0, 20.0 ],
					"numoutlets" : 0
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Change a parameter index",
					"fontname" : "Arial",
					"id" : "obj-16",
					"fontsize" : 12.0,
					"bgcolor" : [ 0.678431, 0.819608, 0.819608, 1.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 211.5, 433.0, 157.0, 20.0 ],
					"numoutlets" : 0
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "number",
					"fontname" : "Arial",
					"outlettype" : [ "int", "bang" ],
					"id" : "obj-17",
					"minimum" : 0,
					"fontsize" : 12.0,
					"maximum" : 8,
					"numinlets" : 1,
					"patching_rect" : [ 160.0, 433.0, 50.0, 20.0 ],
					"numoutlets" : 2
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "pak param 0 0.5",
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"id" : "obj-18",
					"fontsize" : 12.0,
					"numinlets" : 3,
					"patching_rect" : [ 120.0, 498.0, 99.0, 20.0 ],
					"numoutlets" : 1
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "flonum",
					"fontname" : "Arial",
					"outlettype" : [ "float", "bang" ],
					"id" : "obj-19",
					"minimum" : 0.0,
					"fontsize" : 12.0,
					"maximum" : 1.0,
					"numinlets" : 1,
					"patching_rect" : [ 200.0, 465.0, 50.0, 20.0 ],
					"numoutlets" : 2
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "jit.gl.freeframe twindow @loadeffect FFGLHeat @param 0 0.5",
					"fontname" : "Arial",
					"outlettype" : [ "", "" ],
					"id" : "obj-10",
					"color" : [ 0.239216, 0.643137, 0.709804, 1.0 ],
					"fontsize" : 12.0,
					"numinlets" : 1,
					"patching_rect" : [ 67.0, 739.0, 339.0, 20.0 ],
					"numoutlets" : 2
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "jit.gl.videoplane twindow @transform_reset 2 @enable 1",
					"fontname" : "Arial",
					"id" : "obj-11",
					"fontsize" : 12.0,
					"numinlets" : 1,
					"patching_rect" : [ 67.0, 869.0, 311.0, 20.0 ],
					"numoutlets" : 0
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "<  CLICK GO",
					"fontname" : "Arial",
					"id" : "obj-4",
					"fontface" : 1,
					"fontsize" : 16.0,
					"bgcolor" : [ 0.533333, 0.74902, 0.533333, 1.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 516.0, 91.0, 122.0, 25.0 ],
					"numoutlets" : 0
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Utilize FreeFrameGL effects",
					"fontname" : "Arial",
					"id" : "obj-22",
					"fontface" : 2,
					"fontsize" : 14.0,
					"numinlets" : 1,
					"patching_rect" : [ 271.5, 44.0, 197.0, 23.0 ],
					"numoutlets" : 0
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "jit.gl.freeframe",
					"fontname" : "Arial",
					"id" : "obj-21",
					"fontface" : 1,
					"fontsize" : 24.0,
					"bgcolor" : [ 0.827451, 0.827451, 0.827451, 1.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 274.5, 3.0, 181.0, 34.0 ],
					"numoutlets" : 0
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "read dozer.mov",
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"id" : "obj-2",
					"fontface" : 1,
					"fontsize" : 16.0,
					"numinlets" : 2,
					"patching_rect" : [ 67.0, 85.0, 128.0, 23.0 ],
					"numoutlets" : 1
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "jit.pwindow",
					"outlettype" : [ "", "" ],
					"id" : "obj-1",
					"numinlets" : 1,
					"patching_rect" : [ 282.5, 190.0, 171.0, 120.0 ],
					"numoutlets" : 2,
					"name" : "twindow"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "t b b erase",
					"fontname" : "Arial",
					"outlettype" : [ "bang", "bang", "erase" ],
					"id" : "obj-69",
					"fontsize" : 11.595187,
					"numinlets" : 1,
					"patching_rect" : [ 488.0, 171.0, 65.0, 20.0 ],
					"numoutlets" : 3
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "jit.gl.render twindow",
					"fontname" : "Arial",
					"id" : "obj-72",
					"fontsize" : 11.595187,
					"numinlets" : 1,
					"patching_rect" : [ 488.0, 247.0, 113.0, 20.0 ],
					"numoutlets" : 0
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "name twindow",
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"id" : "obj-74",
					"fontsize" : 11.595187,
					"numinlets" : 2,
					"patching_rect" : [ 282.5, 158.0, 85.0, 18.0 ],
					"numoutlets" : 1
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "jit.pwindow",
					"outlettype" : [ "", "" ],
					"id" : "obj-12",
					"numinlets" : 1,
					"patching_rect" : [ 93.0, 190.0, 171.0, 120.0 ],
					"numoutlets" : 2
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "jit.qt.movie @drawto twindow @output_texture 1",
					"linecount" : 2,
					"fontname" : "Arial",
					"outlettype" : [ "jit_matrix", "" ],
					"id" : "obj-5",
					"fontsize" : 12.0,
					"numinlets" : 1,
					"patching_rect" : [ 67.0, 132.0, 159.0, 34.0 ],
					"numoutlets" : 2
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "qmetro 33",
					"fontname" : "Arial",
					"outlettype" : [ "bang" ],
					"id" : "obj-3",
					"fontsize" : 12.0,
					"numinlets" : 2,
					"patching_rect" : [ 488.0, 128.0, 65.0, 20.0 ],
					"numoutlets" : 1
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "toggle",
					"outlettype" : [ "int" ],
					"id" : "obj-6",
					"bgcolor" : [ 0.439216, 0.74902, 0.254902, 1.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 488.0, 91.0, 25.0, 25.0 ],
					"numoutlets" : 1
				}

			}
 ],
		"lines" : [ 			{
				"patchline" : 				{
					"source" : [ "obj-53", 0 ],
					"destination" : [ "obj-10", 0 ],
					"hidden" : 0,
					"midpoints" : [ 102.5, 701.5, 76.5, 701.5 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-74", 0 ],
					"destination" : [ "obj-1", 0 ],
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
					"midpoints" : [ 543.5, 196.5, 497.5, 196.5 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-69", 1 ],
					"destination" : [ "obj-29", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-6", 0 ],
					"destination" : [ "obj-3", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-53", 1 ],
					"destination" : [ "obj-36", 0 ],
					"hidden" : 0,
					"midpoints" : [ 116.5, 673.0, 156.5, 673.0, 156.5, 634.0, 196.5, 634.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-5", 0 ],
					"destination" : [ "obj-12", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-5", 0 ],
					"destination" : [ "obj-10", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-49", 0 ],
					"destination" : [ "obj-10", 0 ],
					"hidden" : 0,
					"midpoints" : [ 125.5, 856.0, 50.0, 856.0, 50.0, 729.0, 76.5, 729.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-45", 0 ],
					"destination" : [ "obj-53", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-43", 0 ],
					"destination" : [ "obj-45", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-39", 0 ],
					"destination" : [ "obj-10", 0 ],
					"hidden" : 0,
					"midpoints" : [ 288.0, 561.0, 76.5, 561.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-37", 0 ],
					"destination" : [ "obj-32", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-36", 0 ],
					"destination" : [ "obj-32", 0 ],
					"hidden" : 0,
					"midpoints" : [ 196.5, 664.5, 243.0, 664.5 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-35", 0 ],
					"destination" : [ "obj-10", 0 ],
					"hidden" : 0,
					"midpoints" : [ 319.0, 732.0, 76.5, 732.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-32", 1 ],
					"destination" : [ "obj-35", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-31", 0 ],
					"destination" : [ "obj-37", 0 ],
					"hidden" : 0,
					"midpoints" : [ 243.0, 635.5, 243.0, 635.5 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-30", 0 ],
					"destination" : [ "obj-5", 0 ],
					"hidden" : 0,
					"midpoints" : [ 20.5, 122.5, 76.5, 122.5 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-3", 0 ],
					"destination" : [ "obj-69", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-28", 0 ],
					"destination" : [ "obj-74", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-20", 0 ],
					"destination" : [ "obj-10", 0 ],
					"hidden" : 0,
					"midpoints" : [ 128.5, 561.0, 76.5, 561.0 ]
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
					"source" : [ "obj-19", 0 ],
					"destination" : [ "obj-18", 2 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-18", 0 ],
					"destination" : [ "obj-10", 0 ],
					"hidden" : 0,
					"midpoints" : [ 129.5, 560.0, 76.5, 560.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-17", 0 ],
					"destination" : [ "obj-18", 1 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-13", 0 ],
					"destination" : [ "obj-49", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-10", 1 ],
					"destination" : [ "obj-31", 0 ],
					"hidden" : 0,
					"midpoints" : [ 396.5, 773.0, 440.0, 773.0, 440.0, 570.0, 243.0, 570.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-10", 1 ],
					"destination" : [ "obj-14", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-10", 0 ],
					"destination" : [ "obj-11", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-26", 0 ],
					"destination" : [ "obj-2", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
 ]
	}

}
