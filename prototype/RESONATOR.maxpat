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
		"rect" : [ 34.0, 77.0, 1852.0, 929.0 ],
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
					"id" : "obj-10",
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
						"rect" : [ 176.0, 85.0, 906.0, 920.0 ],
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
									"id" : "obj-6",
									"maxclass" : "newobj",
									"numinlets" : 1,
									"numoutlets" : 0,
									"patching_rect" : [ 18.0, 883.0, 35.0, 22.0 ],
									"text" : "out 1"
								}

							}
, 							{
								"box" : 								{
									"code" : "// Parameters (adjust via UI if needed)\r\nParam f(440, min=20, max=20000, default=440);             // Fundamental frequency (Hz)\r\nParam bw(100, min=1, max=200, default=100);            \t  // Bandwidth (Hz) controlling damping\r\nParam overtone_gain(0.5, min=0, max=1, default=0.5);      // Overtone peak height factor (0 to 1)\r\nParam n(3, min=1, max=8, default=3);                      // Number of overtones\r\n\r\n// Two delay lines (history objects)\r\nHistory d1(0);\r\nHistory d2(0);\r\n\r\n// Get the samplerate provided by Gen~\r\nfs = SAMPLERATE;\r\n\r\n\r\n// Compute digital frequency and pole radius\r\nomega = 2 * 3.14159265 * f / fs;\r\nr = exp(-3.14159265 * bw / fs);\r\n\r\n// Calculate denominator coefficients for a 2nd-order resonator:\r\n// Poles at r * exp(±j * omega)\r\na1 = 2 * r * cos(omega);\r\na2 = - (r * r);\r\n\r\n// Calculate the numerator gain M corresponding to the finite series:\r\n// 1 + overtone_gain * z^(-1) + ... + overtone_gain^n * z^(-n)\r\n// Special handling when overtone_gain is 1.\r\nM = 0;\r\nif (abs(overtone_gain - 1) < 1e-6) {\r\n    M = n + 1;\r\n} else {\r\n    M = (1 - pow(overtone_gain, n + 1)) / (1 - overtone_gain);\r\n}\r\n\r\n// Process the input sample (in1) using the resonator:\r\ny_den = in1 + a1 * d1 + a2 * d2;\r\ny = M * y_den;\r\n\r\n// Update the delay lines\r\nd2 = d1;\r\nd1 = y;\r\n\r\n// Output the result\r\nout1 = y;",
									"fontface" : 0,
									"fontname" : "<Monospaced>",
									"fontsize" : 12.0,
									"id" : "obj-5",
									"maxclass" : "codebox",
									"numinlets" : 1,
									"numoutlets" : 1,
									"outlettype" : [ "" ],
									"patching_rect" : [ 18.0, 40.0, 802.0, 829.0 ]
								}

							}
, 							{
								"box" : 								{
									"id" : "obj-1",
									"maxclass" : "newobj",
									"numinlets" : 0,
									"numoutlets" : 1,
									"outlettype" : [ "" ],
									"patching_rect" : [ 18.0, 7.0, 28.0, 22.0 ],
									"text" : "in 1"
								}

							}
 ],
						"lines" : [ 							{
								"patchline" : 								{
									"destination" : [ "obj-5", 0 ],
									"source" : [ "obj-1", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-6", 0 ],
									"source" : [ "obj-5", 0 ]
								}

							}
 ]
					}
,
					"patching_rect" : [ 83.0, 267.0, 36.0, 22.0 ],
					"text" : "gen~"
				}

			}
, 			{
				"box" : 				{
					"floatoutput" : 1,
					"id" : "obj-22",
					"maxclass" : "slider",
					"numinlets" : 1,
					"numoutlets" : 1,
					"orientation" : 1,
					"outlettype" : [ "" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 585.0, 324.0, 150.0, 25.0 ],
					"size" : 1.0
				}

			}
, 			{
				"box" : 				{
					"floatoutput" : 1,
					"id" : "obj-21",
					"maxclass" : "slider",
					"numinlets" : 1,
					"numoutlets" : 1,
					"orientation" : 1,
					"outlettype" : [ "" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 585.0, 217.0, 150.0, 25.0 ],
					"size" : 1.0
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-47",
					"logfreq" : 1,
					"maxclass" : "spectroscope~",
					"monochrome" : 0,
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 375.0, 433.0, 596.0, 430.0 ],
					"range" : [ 0.0, 1.0 ],
					"sono" : 1
				}

			}
, 			{
				"box" : 				{
					"fontsize" : 12.0,
					"id" : "obj-9",
					"lastchannelcount" : 0,
					"maxclass" : "live.gain~",
					"numinlets" : 2,
					"numoutlets" : 5,
					"orientation" : 1,
					"outlettype" : [ "signal", "signal", "", "float", "list" ],
					"parameter_enable" : 1,
					"patching_rect" : [ 173.0, 324.0, 129.0, 39.0 ],
					"saved_attribute_attributes" : 					{
						"valueof" : 						{
							"parameter_initial" : [ -70 ],
							"parameter_initial_enable" : 1,
							"parameter_longname" : "live.gain~[1]",
							"parameter_mmax" : 6.0,
							"parameter_mmin" : -70.0,
							"parameter_modmode" : 0,
							"parameter_shortname" : "live.gain~",
							"parameter_type" : 0,
							"parameter_unitstyle" : 4
						}

					}
,
					"showname" : 0,
					"varname" : "live.gain~[1]"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-17",
					"maxclass" : "slider",
					"numinlets" : 1,
					"numoutlets" : 1,
					"orientation" : 1,
					"outlettype" : [ "" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 397.0, 324.0, 150.0, 25.0 ],
					"size" : 2000.0
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-16",
					"knobshape" : 4,
					"maxclass" : "slider",
					"numinlets" : 1,
					"numoutlets" : 1,
					"orientation" : 1,
					"outlettype" : [ "" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 397.0, 216.0, 150.0, 26.0 ]
				}

			}
, 			{
				"box" : 				{
					"bubble" : 1,
					"fontname" : "Arial",
					"fontsize" : 13.0,
					"id" : "obj-11",
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 229.0, 100.0, 147.0, 25.0 ],
					"text" : "play/stop current file"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-8",
					"maxclass" : "toggle",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "int" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 203.0, 101.0, 24.0, 24.0 ]
				}

			}
, 			{
				"box" : 				{
					"fontsize" : 12.0,
					"id" : "obj-5",
					"lastchannelcount" : 0,
					"maxclass" : "live.gain~",
					"numinlets" : 2,
					"numoutlets" : 5,
					"orientation" : 1,
					"outlettype" : [ "signal", "signal", "", "float", "list" ],
					"parameter_enable" : 1,
					"patching_rect" : [ 173.0, 208.0, 129.0, 39.0 ],
					"saved_attribute_attributes" : 					{
						"valueof" : 						{
							"parameter_initial" : [ -70 ],
							"parameter_initial_enable" : 1,
							"parameter_longname" : "live.gain~",
							"parameter_mmax" : 6.0,
							"parameter_mmin" : -70.0,
							"parameter_modmode" : 0,
							"parameter_shortname" : "live.gain~",
							"parameter_type" : 0,
							"parameter_unitstyle" : 4
						}

					}
,
					"showname" : 0,
					"varname" : "live.gain~"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-7",
					"local" : 1,
					"maxclass" : "ezdac~",
					"numinlets" : 2,
					"numoutlets" : 0,
					"patching_rect" : [ 173.0, 485.0, 44.0, 44.0 ],
					"prototypename" : "helpfile"
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 13.0,
					"id" : "obj-43",
					"maxclass" : "newobj",
					"numinlets" : 2,
					"numoutlets" : 3,
					"outlettype" : [ "signal", "signal", "bang" ],
					"patching_rect" : [ 174.0, 162.0, 238.0, 23.0 ],
					"text" : "sfplay~ 2"
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 13.0,
					"id" : "obj-41",
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 156.0, 101.0, 40.0, 23.0 ],
					"text" : "open"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-1",
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
						"rect" : [ 801.0, 91.0, 982.0, 913.0 ],
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
									"code" : "// Parameters\r\nParam freq(0, max=20000, default=440, min=20);\r\nParam bandwidth(0, min=0, default=100);\r\nParam harmonics(0, 1, default=0.5);\r\nParam drive(0, 1, default=0); // New saturation control\r\n\r\n// History variables\r\nHistory history1_1(0), history2_1(0); // Fundamental\r\nHistory history1_2(0), history2_2(0); // 2nd harmonic\r\nHistory history1_3(0), history2_3(0); // 3rd harmonic\r\nHistory history1_4(0), history2_4(0); // 4th harmonic\r\nHistory history1_5(0), history2_5(0); // 5th harmonic\r\n\r\n// Shared calculations\r\nbandwidthRadians = (bandwidth / SAMPLERATE) * twopi;\r\nfundamentalOutput = 0;\r\novertoneSum = 0;\r\n\r\n\r\n//forward declarations for conditionally declared stuff\r\nnew_h1 = 0;\r\nnew_h2 = 0;\r\nh1 = 0;\r\nh2 = 0;\r\n\r\n// Process fundamental (n=1) separately\r\ncurrentFreq = freq;\r\nif(currentFreq < SAMPLERATE/2) {\r\n    freqRadians = (currentFreq/SAMPLERATE) * twopi;\r\n    peakRadius = 1 - (bandwidthRadians/2);\r\n    peakLocation = acos((2 * peakRadius * cos(freqRadians))/(1 + pow(peakRadius, 2)));\r\n    normFactor = (1 - pow(peakRadius, 2)) * sin(peakLocation) * 1.4;\r\n    \r\n    // Process fundamental\r\n    out_n = (in1 * normFactor) + \r\n           (2 * peakRadius * cos(peakLocation) * history2_1) - \r\n           (history1_1 * pow(peakRadius, 2));\r\n    \r\n    // Update history\r\n    new_h1 = fixdenorm(history2_1);\r\n    new_h2 = fixdenorm(out_n);\r\n    fundamentalOutput = out_n;\r\n} else {\r\n    new_h1 = history1_1;\r\n    new_h2 = history2_1;\r\n}\r\n\r\n// Update fundamental history\r\nhistory1_1 = new_h1; \r\nhistory2_1 = new_h2;\r\n\r\n// Process 4 overtones (n=2-5)\r\nfor(n=2; n<=5; n+=1) {\r\n    currentFreq = freq * n;\r\n    if(currentFreq >= SAMPLERATE/2) continue;\r\n    \r\n    // Get appropriate history\r\n    if(n == 2) { h1 = history1_2; h2 = history2_2; }\r\n    if(n == 3) { h1 = history1_3; h2 = history2_3; }\r\n    if(n == 4) { h1 = history1_4; h2 = history2_4; }\r\n    if(n == 5) { h1 = history1_5; h2 = history2_5; }\r\n    \r\n    // Calculate coefficients\r\n    freqRadians = (currentFreq/SAMPLERATE) * twopi;\r\n    peakRadius = 1 - (bandwidthRadians/2);\r\n    peakLocation = acos((2 * peakRadius * cos(freqRadians))/(1 + pow(peakRadius, 2)));\r\n    normFactor = (1 - pow(peakRadius, 2)) * sin(peakLocation) * 1.4;\r\n    \r\n    // Process overtone\r\n    out_n = (in1 * normFactor) + \r\n           (2 * peakRadius * cos(peakLocation) * h2) - \r\n           (h1 * pow(peakRadius, 2));\r\n    \r\n    // Calculate weight (emphasize higher harmonics)\r\n    weight = harmonics * (n-1); // Linear scaling\r\n    \r\n    // Denormal protection and store updates\r\n    if(n == 2) { \r\n        history1_2 = fixdenorm(h2); \r\n        history2_2 = fixdenorm(out_n); \r\n    }\r\n    if(n == 3) { \r\n        history1_3 = fixdenorm(h2); \r\n        history2_3 = fixdenorm(out_n); \r\n    }\r\n    if(n == 4) { \r\n        history1_4 = fixdenorm(h2); \r\n        history2_4 = fixdenorm(out_n); \r\n    }\r\n    if(n == 5) { \r\n        history1_5 = fixdenorm(h2); \r\n        history2_5 = fixdenorm(out_n); \r\n    }\r\n    \r\n    overtoneSum += out_n * weight;\r\n}\r\n\r\n// Combine outputs with saturation\r\ntotalOut = fundamentalOutput + overtoneSum;\r\n\r\n// Soft-clipping nonlinearity\r\nsaturated = tanh(totalOut * (1 + (drive * 3))) / (1 + (drive * 3));\r\nout1 = mix(totalOut, saturated, drive);",
									"fontface" : 0,
									"fontname" : "<Monospaced>",
									"fontsize" : 12.0,
									"id" : "obj-62",
									"maxclass" : "codebox",
									"numinlets" : 1,
									"numoutlets" : 1,
									"outlettype" : [ "" ],
									"patching_rect" : [ 26.0, 78.0, 775.0, 772.0 ]
								}

							}
, 							{
								"box" : 								{
									"id" : "obj-1",
									"maxclass" : "newobj",
									"numinlets" : 0,
									"numoutlets" : 1,
									"outlettype" : [ "" ],
									"patching_rect" : [ 26.0, 35.0, 28.0, 22.0 ],
									"text" : "in 1"
								}

							}
, 							{
								"box" : 								{
									"id" : "obj-4",
									"maxclass" : "newobj",
									"numinlets" : 1,
									"numoutlets" : 0,
									"patching_rect" : [ 26.0, 863.0, 35.0, 22.0 ],
									"text" : "out 1"
								}

							}
 ],
						"lines" : [ 							{
								"patchline" : 								{
									"destination" : [ "obj-62", 0 ],
									"source" : [ "obj-1", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-4", 0 ],
									"source" : [ "obj-62", 0 ]
								}

							}
 ]
					}
,
					"patching_rect" : [ 173.0, 267.0, 36.0, 22.0 ],
					"text" : "gen~"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-3",
					"linecount" : 7,
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 633.0, 61.0, 155.0, 103.0 ],
					"text" : "Resonance: \"the tendency of a system to steal energy from, and vibrate sympathetically at, a particular frequency in response to energy supplied at the frequency\""
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-6",
					"linecount" : 2,
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 807.0, 299.0, 153.0, 34.0 ],
					"text" : "add nonlinear feedback (soft clipping or saturation) "
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-4",
					"linecount" : 19,
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 810.0, 20.0, 150.0, 269.0 ],
					"text" : "Use a bank of resonators to add harmonic complexity.\n\nOne fundamental resonator, and then resonators at 2x, 3x, 4x, etc. frequencies to add harmonic resonance.\n\ntie all filter frequencies to a  common base freq so that the harmonics shift with the fundamental\n\nblend outputs with adjustable gains to emphasize/de-emphasize specific harmonies"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-2",
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 632.0, 25.0, 153.0, 20.0 ]
				}

			}
, 			{
				"box" : 				{
					"attr" : "bandwidth",
					"id" : "obj-13",
					"maxclass" : "attrui",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 397.0, 257.0, 150.0, 22.0 ]
				}

			}
, 			{
				"box" : 				{
					"attr" : "freq",
					"id" : "obj-14",
					"maxclass" : "attrui",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 397.0, 362.0, 150.0, 22.0 ]
				}

			}
, 			{
				"box" : 				{
					"attr" : "harmonics",
					"id" : "obj-18",
					"maxclass" : "attrui",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 585.0, 257.0, 150.0, 22.0 ]
				}

			}
, 			{
				"box" : 				{
					"attr" : "drive",
					"id" : "obj-19",
					"maxclass" : "attrui",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 585.0, 362.0, 150.0, 22.0 ]
				}

			}
 ],
		"lines" : [ 			{
				"patchline" : 				{
					"destination" : [ "obj-9", 1 ],
					"order" : 0,
					"source" : [ "obj-1", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-9", 0 ],
					"order" : 1,
					"source" : [ "obj-1", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-1", 0 ],
					"midpoints" : [ 406.5, 305.0, 322.5, 305.0, 322.5, 256.0, 182.5, 256.0 ],
					"source" : [ "obj-13", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-1", 0 ],
					"midpoints" : [ 406.5, 394.0, 335.5, 394.0, 335.5, 256.0, 182.5, 256.0 ],
					"source" : [ "obj-14", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-13", 0 ],
					"source" : [ "obj-16", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-14", 0 ],
					"source" : [ "obj-17", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-1", 0 ],
					"midpoints" : [ 594.5, 292.0, 352.5, 292.0, 352.5, 256.0, 182.5, 256.0 ],
					"source" : [ "obj-18", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-1", 0 ],
					"midpoints" : [ 594.5, 394.0, 374.5, 394.0, 374.5, 256.0, 182.5, 256.0 ],
					"source" : [ "obj-19", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-18", 0 ],
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
					"destination" : [ "obj-43", 0 ],
					"source" : [ "obj-41", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-5", 1 ],
					"order" : 0,
					"source" : [ "obj-43", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-5", 0 ],
					"order" : 1,
					"source" : [ "obj-43", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-1", 0 ],
					"source" : [ "obj-5", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-43", 0 ],
					"source" : [ "obj-8", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-47", 0 ],
					"order" : 0,
					"source" : [ "obj-9", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-7", 1 ],
					"order" : 1,
					"source" : [ "obj-9", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-7", 0 ],
					"order" : 2,
					"source" : [ "obj-9", 0 ]
				}

			}
 ],
		"parameters" : 		{
			"obj-5" : [ "live.gain~", "live.gain~", 0 ],
			"obj-9" : [ "live.gain~[1]", "live.gain~", 0 ],
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
