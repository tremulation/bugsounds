{
	"patcher" : 	{
		"fileversion" : 1,
		"appversion" : 		{
			"major" : 8,
			"minor" : 6,
			"revision" : 2,
			"architecture" : "x64",
			"modernui" : 1
		}
,
		"classnamespace" : "box",
		"rect" : [ 35.0, 85.0, 890.0, 913.0 ],
		"bglocked" : 0,
		"openinpresentation" : 0,
		"default_fontsize" : 12.0,
		"default_fontface" : 0,
		"default_fontname" : "Arial",
		"gridonopen" : 1,
		"gridsize" : [ 15.0, 15.0 ],
		"gridsnaponopen" : 1,
		"objectsnaponopen" : 1,
		"statusbarvisible" : 2,
		"toolbarvisible" : 1,
		"lefttoolbarpinned" : 0,
		"toptoolbarpinned" : 0,
		"righttoolbarpinned" : 0,
		"bottomtoolbarpinned" : 0,
		"toolbars_unpinned_last_save" : 0,
		"tallnewobj" : 0,
		"boxanimatetime" : 200,
		"enablehscroll" : 1,
		"enablevscroll" : 1,
		"devicewidth" : 0.0,
		"description" : "",
		"digest" : "",
		"tags" : "",
		"style" : "",
		"subpatcher_template" : "",
		"assistshowspatchername" : 0,
		"boxes" : [ 			{
				"box" : 				{
					"id" : "obj-30",
					"lastchannelcount" : 0,
					"maxclass" : "live.gain~",
					"numinlets" : 2,
					"numoutlets" : 5,
					"outlettype" : [ "signal", "signal", "", "float", "list" ],
					"parameter_enable" : 1,
					"patching_rect" : [ 73.0, 266.0, 48.0, 136.0 ],
					"saved_attribute_attributes" : 					{
						"valueof" : 						{
							"parameter_longname" : "live.gain~[1]",
							"parameter_mmax" : 6.0,
							"parameter_mmin" : -70.0,
							"parameter_modmode" : 3,
							"parameter_shortname" : "live.gain~",
							"parameter_type" : 0,
							"parameter_unitstyle" : 4
						}

					}
,
					"varname" : "live.gain~[1]"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-29",
					"knobshape" : 5,
					"maxclass" : "slider",
					"min" : 1.0,
					"numinlets" : 1,
					"numoutlets" : 1,
					"orientation" : 1,
					"outlettype" : [ "" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 415.0, 85.0, 150.0, 22.5 ],
					"size" : 8.0
				}

			}
, 			{
				"box" : 				{
					"floatoutput" : 1,
					"id" : "obj-26",
					"knobshape" : 5,
					"maxclass" : "slider",
					"min" : 1.0,
					"numinlets" : 1,
					"numoutlets" : 1,
					"orientation" : 1,
					"outlettype" : [ "" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 255.0, 85.0, 150.0, 22.5 ],
					"size" : 10.0
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-22",
					"knobshape" : 5,
					"maxclass" : "slider",
					"numinlets" : 1,
					"numoutlets" : 1,
					"orientation" : 1,
					"outlettype" : [ "" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 415.0, 158.0, 150.0, 22.5 ],
					"size" : 2000.0
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-21",
					"knobshape" : 5,
					"maxclass" : "slider",
					"min" : 20.0,
					"numinlets" : 1,
					"numoutlets" : 1,
					"orientation" : 1,
					"outlettype" : [ "" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 255.0, 158.0, 150.0, 22.5 ],
					"size" : 10000.0
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-15",
					"maxclass" : "toggle",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "int" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 220.0, 146.0, 24.0, 24.0 ]
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-4",
					"linecount" : 10,
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 582.0, 149.0, 268.0, 144.0 ],
					"text" : "None of this shit you can see right now matters.  (ctrl + click the gen object to see the resonator code)\n\nhit open to put a sample in the sfplay\n\nset the freq and Q\n\nand then toggle the thing on to run a sample through"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-31",
					"logfreq" : 1,
					"maxclass" : "spectroscope~",
					"monochrome" : 0,
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 267.0, 323.0, 387.0, 136.0 ],
					"range" : [ 0.0, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-18",
					"maxclass" : "ezdac~",
					"numinlets" : 2,
					"numoutlets" : 0,
					"patching_rect" : [ 183.0, 486.0, 45.0, 45.0 ]
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-17",
					"lastchannelcount" : 0,
					"maxclass" : "live.gain~",
					"numinlets" : 2,
					"numoutlets" : 5,
					"outlettype" : [ "signal", "signal", "", "float", "list" ],
					"parameter_enable" : 1,
					"patching_rect" : [ 183.0, 323.0, 48.0, 136.0 ],
					"saved_attribute_attributes" : 					{
						"valueof" : 						{
							"parameter_longname" : "live.gain~",
							"parameter_mmax" : 6.0,
							"parameter_mmin" : -70.0,
							"parameter_modmode" : 3,
							"parameter_shortname" : "live.gain~",
							"parameter_type" : 0,
							"parameter_unitstyle" : 4
						}

					}
,
					"varname" : "live.gain~"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-12",
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 182.0, 147.0, 35.0, 22.0 ],
					"text" : "open"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-9",
					"maxclass" : "newobj",
					"numinlets" : 2,
					"numoutlets" : 3,
					"outlettype" : [ "signal", "signal", "bang" ],
					"patching_rect" : [ 182.0, 192.0, 57.0, 22.0 ],
					"text" : "sfplay~ 2"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-139",
					"maxclass" : "newobj",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "signal" ],
					"patcher" : 					{
						"fileversion" : 1,
						"appversion" : 						{
							"major" : 8,
							"minor" : 6,
							"revision" : 2,
							"architecture" : "x64",
							"modernui" : 1
						}
,
						"classnamespace" : "dsp.gen",
						"rect" : [ 994.0, 85.0, 889.0, 924.0 ],
						"bglocked" : 0,
						"openinpresentation" : 0,
						"default_fontsize" : 12.0,
						"default_fontface" : 0,
						"default_fontname" : "Arial",
						"gridonopen" : 1,
						"gridsize" : [ 15.0, 15.0 ],
						"gridsnaponopen" : 1,
						"objectsnaponopen" : 1,
						"statusbarvisible" : 2,
						"toolbarvisible" : 1,
						"lefttoolbarpinned" : 0,
						"toptoolbarpinned" : 0,
						"righttoolbarpinned" : 0,
						"bottomtoolbarpinned" : 0,
						"toolbars_unpinned_last_save" : 0,
						"tallnewobj" : 0,
						"boxanimatetime" : 200,
						"enablehscroll" : 1,
						"enablevscroll" : 1,
						"devicewidth" : 0.0,
						"description" : "",
						"digest" : "",
						"tags" : "",
						"style" : "",
						"subpatcher_template" : "",
						"assistshowspatchername" : 0,
						"boxes" : [ 							{
								"box" : 								{
									"code" : "// Parameters\r\nParam freq(440, 20, 20000);         // Fundamental frequency in Hz: default 440, range 20–20000 Hz\r\nParam bandwidth(50, 1, 2000);       // Bandwidth in Hz: default 50, range 1–2000 Hz\r\nParam gain(1, 0, 10);             // Gain: default 1, range 0–10\r\nParam overtoneNum(4, 1, 8);       // Number of resonators: default 4, range 1–8\r\n\r\n// History objects for each of the 8 resonators (each with two delay states)\r\nHistory h1_1(0); History h2_1(0);\r\nHistory h1_2(0); History h2_2(0);\r\nHistory h1_3(0); History h2_3(0);\r\nHistory h1_4(0); History h2_4(0);\r\nHistory h1_5(0); History h2_5(0);\r\nHistory h1_6(0); History h2_6(0);\r\nHistory h1_7(0); History h2_7(0);\r\nHistory h1_8(0); History h2_8(0);\r\n\r\n// Load the incoming sample into accumulator\r\nacc = in1;\r\n\r\n// Loop over each resonator (0-indexed)\r\nfor(i = 0; i < overtoneNum; i += 1) {\r\n    // Calculate the resonant frequency for this overtone\r\n    resonantFreq = freq * (i + 1);\r\n    omega = 2 * PI * resonantFreq / samplerate;\r\n    \r\n    // Compute the pole radius based on desired bandwidth (in Hz)\r\n    r = exp(-PI * bandwidth / samplerate);\r\n    coeff = 2 * r * cos(omega);\r\n    \r\n    // Temporary variable for current resonator's output\r\n    temp = 0;\r\n    \r\n    // Select the appropriate history pair for this resonator\r\n    if(i == 0) {\r\n        temp = coeff * h1_1 - (r * r) * h2_1 + gain * acc;\r\n        h2_1 = h1_1;\r\n        h1_1 = temp;\r\n    } else if(i == 1) {\r\n        temp = coeff * h1_2 - (r * r) * h2_2 + gain * acc;\r\n        h2_2 = h1_2;\r\n        h1_2 = temp;\r\n    } else if(i == 2) {\r\n        temp = coeff * h1_3 - (r * r) * h2_3 + gain * acc;\r\n        h2_3 = h1_3;\r\n        h1_3 = temp;\r\n    } else if(i == 3) {\r\n        temp = coeff * h1_4 - (r * r) * h2_4 + gain * acc;\r\n        h2_4 = h1_4;\r\n        h1_4 = temp;\r\n    } else if(i == 4) {\r\n        temp = coeff * h1_5 - (r * r) * h2_5 + gain * acc;\r\n        h2_5 = h1_5;\r\n        h1_5 = temp;\r\n    } else if(i == 5) {\r\n        temp = coeff * h1_6 - (r * r) * h2_6 + gain * acc;\r\n        h2_6 = h1_6;\r\n        h1_6 = temp;\r\n    } else if(i == 6) {\r\n        temp = coeff * h1_7 - (r * r) * h2_7 + gain * acc;\r\n        h2_7 = h1_7;\r\n        h1_7 = temp;\r\n    } else if(i == 7) {\r\n        temp = coeff * h1_8 - (r * r) * h2_8 + gain * acc;\r\n        h2_8 = h1_8;\r\n        h1_8 = temp;\r\n    }\r\n    \r\n    // Update the accumulator with this resonator's output\r\n    acc = temp;\r\n}\r\n\r\n// Output the final accumulated value\r\nout1 = acc;",
									"fontface" : 0,
									"fontname" : "<Monospaced>",
									"fontsize" : 12.0,
									"id" : "obj-5",
									"maxclass" : "codebox",
									"numinlets" : 1,
									"numoutlets" : 1,
									"outlettype" : [ "" ],
									"patching_rect" : [ 24.0, 49.0, 803.0, 758.0 ]
								}

							}
, 							{
								"box" : 								{
									"id" : "obj-61",
									"linecount" : 5,
									"maxclass" : "comment",
									"numinlets" : 1,
									"numoutlets" : 0,
									"patching_rect" : [ 449.0, 68.0, 207.0, 75.0 ],
									"text" : "y(nT) = Ax(nT)+By(nT-T)+Cy(nT-2T)\n\nC = -exp(-2PI BW T)\nB = 2exp(-PI BW T)cos(2PI F T)\nA = 1 - B - C"
								}

							}
 ],
						"lines" : [  ],
						"editing_bgcolor" : [ 0.9, 0.9, 0.9, 1.0 ]
					}
,
					"patching_rect" : [ 183.0, 271.0, 123.0, 22.0 ],
					"text" : "gen~ @t resonator"
				}

			}
, 			{
				"box" : 				{
					"attr" : "freq",
					"id" : "obj-16",
					"maxclass" : "attrui",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 255.0, 192.0, 150.0, 22.0 ]
				}

			}
, 			{
				"box" : 				{
					"attr" : "bandwidth",
					"id" : "obj-19",
					"maxclass" : "attrui",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 415.0, 192.0, 150.0, 22.0 ]
				}

			}
, 			{
				"box" : 				{
					"attr" : "gain",
					"id" : "obj-24",
					"maxclass" : "attrui",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 255.0, 122.0, 150.0, 22.0 ]
				}

			}
, 			{
				"box" : 				{
					"attr" : "overtoneNum",
					"id" : "obj-28",
					"maxclass" : "attrui",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 415.0, 122.0, 150.0, 22.0 ]
				}

			}
 ],
		"lines" : [ 			{
				"patchline" : 				{
					"destination" : [ "obj-9", 0 ],
					"source" : [ "obj-12", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-17", 0 ],
					"order" : 1,
					"source" : [ "obj-139", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-31", 0 ],
					"order" : 0,
					"source" : [ "obj-139", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-9", 1 ],
					"source" : [ "obj-15", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-139", 0 ],
					"source" : [ "obj-16", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-18", 1 ],
					"order" : 0,
					"source" : [ "obj-17", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-18", 0 ],
					"order" : 1,
					"source" : [ "obj-17", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-139", 0 ],
					"midpoints" : [ 424.5, 267.0, 192.5, 267.0 ],
					"source" : [ "obj-19", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-16", 0 ],
					"source" : [ "obj-21", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-19", 0 ],
					"source" : [ "obj-22", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-139", 0 ],
					"midpoints" : [ 264.5, 183.0, 246.0, 183.0, 246.0, 168.0, 168.0, 168.0, 168.0, 267.0, 192.5, 267.0 ],
					"source" : [ "obj-24", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-24", 0 ],
					"source" : [ "obj-26", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-139", 0 ],
					"midpoints" : [ 424.5, 183.0, 411.0, 183.0, 411.0, 267.0, 192.5, 267.0 ],
					"source" : [ "obj-28", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-28", 0 ],
					"source" : [ "obj-29", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-139", 0 ],
					"source" : [ "obj-30", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-30", 0 ],
					"source" : [ "obj-9", 0 ]
				}

			}
 ],
		"parameters" : 		{
			"obj-17" : [ "live.gain~", "live.gain~", 0 ],
			"obj-30" : [ "live.gain~[1]", "live.gain~", 0 ],
			"parameterbanks" : 			{
				"0" : 				{
					"index" : 0,
					"name" : "",
					"parameters" : [ "-", "-", "-", "-", "-", "-", "-", "-" ]
				}

			}
,
			"inherited_shortname" : 1
		}
,
		"dependency_cache" : [  ],
		"autosave" : 0
	}

}
