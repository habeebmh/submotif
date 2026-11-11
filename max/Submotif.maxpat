{
  "patcher": {
    "fileversion": 1,
    "appversion": {
      "major": 8,
      "minor": 6,
      "revision": 5,
      "architecture": "x64",
      "modernui": 1
    },
    "classnamespace": "box",
    "rect": [
      80,
      80,
      1260,
      700
    ],
    "openrect": [
      0,
      0,
      1000,
      169
    ],
    "openinpresentation": 1,
    "bglocked": 1,
    "default_fontsize": 11,
    "default_fontname": "Avenir Next",
    "bgcolor": [
      0.09803921568627451,
      0.10980392156862745,
      0.11764705882352941,
      1.0
    ],
    "editing_bgcolor": [
      0.09803921568627451,
      0.10980392156862745,
      0.11764705882352941,
      1.0
    ],
    "locked_bgcolor": [
      0.09803921568627451,
      0.10980392156862745,
      0.11764705882352941,
      1.0
    ],
    "presentation": 1,
    "devicewidth": 1000.0,
    "enablehscroll": 0,
    "enablevscroll": 0,
    "description": "SUBMOTIF by Habeeb — procedural MIDI basslines for any instrument.",
    "digest": "A musical bassline generator with native single-track MIDI output.",
    "tags": "MIDI bass generator house disco funk",
    "boxes": [
      {
        "box": {
          "id": "brand",
          "maxclass": "comment",
          "patching_rect": [
            16,
            5,
            140,
            25
          ],
          "numinlets": 1,
          "numoutlets": 0,
          "presentation": 1,
          "presentation_rect": [
            16,
            5,
            140,
            25
          ],
          "text": "SUBMOTIF",
          "fontsize": 18,
          "fontname": "Avenir Next",
          "textcolor": [
            0.9411764705882353,
            0.9490196078431372,
            0.9294117647058824,
            1.0
          ],
          "fontface": 1,
          "varname": "brand"
        }
      },
      {
        "box": {
          "id": "edition",
          "maxclass": "comment",
          "patching_rect": [
            159,
            10,
            130,
            17
          ],
          "numinlets": 1,
          "numoutlets": 0,
          "presentation": 1,
          "presentation_rect": [
            159,
            10,
            130,
            17
          ],
          "text": "by Habeeb",
          "fontsize": 10,
          "fontname": "Avenir Next",
          "textcolor": [
            0.6549019607843137,
            0.6862745098039216,
            0.6784313725490196,
            1.0
          ],
          "varname": "edition"
        }
      },
      {
        "box": {
          "id": "transport",
          "maxclass": "comment",
          "patching_rect": [
            295,
            10,
            65,
            17
          ],
          "numinlets": 1,
          "numoutlets": 0,
          "presentation": 1,
          "presentation_rect": [
            295,
            10,
            65,
            17
          ],
          "text": "Stopped",
          "fontsize": 10,
          "fontname": "Avenir Next",
          "textcolor": [
            0.6549019607843137,
            0.6862745098039216,
            0.6784313725490196,
            1.0
          ],
          "justification": 2,
          "varname": "transport"
        }
      },
      {
        "box": {
          "id": "summary",
          "maxclass": "comment",
          "patching_rect": [
            380,
            10,
            224,
            18
          ],
          "numinlets": 1,
          "numoutlets": 0,
          "presentation": 1,
          "presentation_rect": [
            380,
            10,
            224,
            18
          ],
          "text": "Ready",
          "fontsize": 11,
          "fontname": "Avenir Next",
          "textcolor": [
            0.9411764705882353,
            0.9490196078431372,
            0.9294117647058824,
            1.0
          ],
          "varname": "summary"
        }
      },
      {
        "box": {
          "id": "status",
          "maxclass": "comment",
          "patching_rect": [
            380,
            145,
            416,
            18
          ],
          "numinlets": 1,
          "numoutlets": 0,
          "presentation": 1,
          "presentation_rect": [
            380,
            145,
            416,
            18
          ],
          "text": "Click a note to keep it. Amber notes are locked.",
          "fontsize": 10,
          "fontname": "Avenir Next",
          "textcolor": [
            0.6549019607843137,
            0.6862745098039216,
            0.6784313725490196,
            1.0
          ],
          "varname": "status"
        }
      },
      {
        "box": {
          "id": "label-root",
          "maxclass": "comment",
          "patching_rect": [
            136,
            31,
            50,
            17
          ],
          "numinlets": 1,
          "numoutlets": 0,
          "presentation": 1,
          "presentation_rect": [
            136,
            31,
            50,
            17
          ],
          "text": "Key",
          "fontsize": 11,
          "fontname": "Avenir Next",
          "textcolor": [
            0.6549019607843137,
            0.6862745098039216,
            0.6784313725490196,
            1.0
          ],
          "hidden": 0,
          "varname": "label-root"
        }
      },
      {
        "box": {
          "id": "root",
          "maxclass": "live.menu",
          "patching_rect": [
            136,
            50,
            50,
            22
          ],
          "numinlets": 1,
          "numoutlets": 3,
          "presentation": 1,
          "presentation_rect": [
            136,
            50,
            50,
            22
          ],
          "varname": "root",
          "parameter_enable": 1,
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Key",
              "parameter_shortname": "Key",
              "parameter_type": 2,
              "parameter_initial_enable": 1,
              "parameter_initial": [
                0
              ],
              "parameter_order": 0,
              "parameter_mmin": 0,
              "parameter_mmax": 11,
              "parameter_enum": [
                "C",
                "C#",
                "D",
                "D#",
                "E",
                "F",
                "F#",
                "G",
                "G#",
                "A",
                "A#",
                "B"
              ]
            }
          },
          "items": [
            "C",
            "C#",
            "D",
            "D#",
            "E",
            "F",
            "F#",
            "G",
            "G#",
            "A",
            "A#",
            "B"
          ],
          "fontsize": 11,
          "fontname": "Avenir Next",
          "bgcolor": [
            0.1411764705882353,
            0.1568627450980392,
            0.16470588235294117,
            1.0
          ],
          "activebgcolor": [
            0.1411764705882353,
            0.1568627450980392,
            0.16470588235294117,
            1.0
          ],
          "textcolor": [
            0.9411764705882353,
            0.9490196078431372,
            0.9294117647058824,
            1.0
          ],
          "activetextcolor": [
            0.9411764705882353,
            0.9490196078431372,
            0.9294117647058824,
            1.0
          ],
          "hidden": 0,
          "hint": ""
        }
      },
      {
        "box": {
          "id": "cfg-root",
          "maxclass": "newobj",
          "patching_rect": [
            20,
            388,
            180,
            22
          ],
          "numinlets": 1,
          "numoutlets": 1,
          "text": "prepend cfg root"
        }
      },
      {
        "box": {
          "id": "label-scale",
          "maxclass": "comment",
          "patching_rect": [
            196,
            31,
            102,
            17
          ],
          "numinlets": 1,
          "numoutlets": 0,
          "presentation": 1,
          "presentation_rect": [
            196,
            31,
            102,
            17
          ],
          "text": "Scale",
          "fontsize": 11,
          "fontname": "Avenir Next",
          "textcolor": [
            0.6549019607843137,
            0.6862745098039216,
            0.6784313725490196,
            1.0
          ],
          "hidden": 0,
          "varname": "label-scale"
        }
      },
      {
        "box": {
          "id": "scale",
          "maxclass": "live.menu",
          "patching_rect": [
            196,
            50,
            102,
            22
          ],
          "numinlets": 1,
          "numoutlets": 3,
          "presentation": 1,
          "presentation_rect": [
            196,
            50,
            102,
            22
          ],
          "varname": "scale",
          "parameter_enable": 1,
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Scale",
              "parameter_shortname": "Scale",
              "parameter_type": 2,
              "parameter_initial_enable": 1,
              "parameter_initial": [
                0
              ],
              "parameter_order": 0,
              "parameter_mmin": 0,
              "parameter_mmax": 4,
              "parameter_enum": [
                "Minor",
                "Dorian",
                "Phrygian",
                "Major",
                "Minor Pent."
              ]
            }
          },
          "items": [
            "Minor",
            "Dorian",
            "Phrygian",
            "Major",
            "Minor Pent."
          ],
          "fontsize": 11,
          "fontname": "Avenir Next",
          "bgcolor": [
            0.1411764705882353,
            0.1568627450980392,
            0.16470588235294117,
            1.0
          ],
          "activebgcolor": [
            0.1411764705882353,
            0.1568627450980392,
            0.16470588235294117,
            1.0
          ],
          "textcolor": [
            0.9411764705882353,
            0.9490196078431372,
            0.9294117647058824,
            1.0
          ],
          "activetextcolor": [
            0.9411764705882353,
            0.9490196078431372,
            0.9294117647058824,
            1.0
          ],
          "hidden": 0,
          "hint": ""
        }
      },
      {
        "box": {
          "id": "cfg-scale",
          "maxclass": "newobj",
          "patching_rect": [
            20,
            460,
            180,
            22
          ],
          "numinlets": 1,
          "numoutlets": 1,
          "text": "prepend cfg scale"
        }
      },
      {
        "box": {
          "id": "label-style",
          "maxclass": "comment",
          "patching_rect": [
            16,
            31,
            110,
            17
          ],
          "numinlets": 1,
          "numoutlets": 0,
          "presentation": 1,
          "presentation_rect": [
            16,
            31,
            110,
            17
          ],
          "text": "Style",
          "fontsize": 11,
          "fontname": "Avenir Next",
          "textcolor": [
            0.6549019607843137,
            0.6862745098039216,
            0.6784313725490196,
            1.0
          ],
          "hidden": 0,
          "varname": "label-style"
        }
      },
      {
        "box": {
          "id": "style",
          "maxclass": "live.menu",
          "patching_rect": [
            16,
            50,
            110,
            22
          ],
          "numinlets": 1,
          "numoutlets": 3,
          "presentation": 1,
          "presentation_rect": [
            16,
            50,
            110,
            22
          ],
          "varname": "style",
          "parameter_enable": 1,
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Style",
              "parameter_shortname": "Style",
              "parameter_type": 2,
              "parameter_initial_enable": 1,
              "parameter_initial": [
                0
              ],
              "parameter_order": 0,
              "parameter_mmin": 0,
              "parameter_mmax": 5,
              "parameter_enum": [
                "Deep House",
                "Rolling",
                "Offbeat",
                "Acid",
                "Disco",
                "Funk"
              ]
            }
          },
          "items": [
            "Deep House",
            "Rolling",
            "Offbeat",
            "Acid",
            "Disco",
            "Funk"
          ],
          "fontsize": 11,
          "fontname": "Avenir Next",
          "bgcolor": [
            0.1411764705882353,
            0.1568627450980392,
            0.16470588235294117,
            1.0
          ],
          "activebgcolor": [
            0.1411764705882353,
            0.1568627450980392,
            0.16470588235294117,
            1.0
          ],
          "textcolor": [
            0.9411764705882353,
            0.9490196078431372,
            0.9294117647058824,
            1.0
          ],
          "activetextcolor": [
            0.9411764705882353,
            0.9490196078431372,
            0.9294117647058824,
            1.0
          ],
          "hidden": 0,
          "hint": ""
        }
      },
      {
        "box": {
          "id": "cfg-style",
          "maxclass": "newobj",
          "patching_rect": [
            20,
            532,
            180,
            22
          ],
          "numinlets": 1,
          "numoutlets": 1,
          "text": "prepend cfg style"
        }
      },
      {
        "box": {
          "id": "label-bars",
          "maxclass": "comment",
          "patching_rect": [
            308,
            31,
            52,
            17
          ],
          "numinlets": 1,
          "numoutlets": 0,
          "presentation": 1,
          "presentation_rect": [
            308,
            31,
            52,
            17
          ],
          "text": "Length",
          "fontsize": 11,
          "fontname": "Avenir Next",
          "textcolor": [
            0.6549019607843137,
            0.6862745098039216,
            0.6784313725490196,
            1.0
          ],
          "hidden": 0,
          "varname": "label-bars"
        }
      },
      {
        "box": {
          "id": "bars",
          "maxclass": "live.menu",
          "patching_rect": [
            308,
            50,
            52,
            22
          ],
          "numinlets": 1,
          "numoutlets": 3,
          "presentation": 1,
          "presentation_rect": [
            308,
            50,
            52,
            22
          ],
          "varname": "bars",
          "parameter_enable": 1,
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Bars",
              "parameter_shortname": "Bars",
              "parameter_type": 2,
              "parameter_initial_enable": 1,
              "parameter_initial": [
                0
              ],
              "parameter_order": 0,
              "parameter_mmin": 0,
              "parameter_mmax": 3,
              "parameter_enum": [
                "1 bar",
                "2 bars",
                "4 bars",
                "8 bars"
              ]
            }
          },
          "items": [
            "1 bar",
            "2 bars",
            "4 bars",
            "8 bars"
          ],
          "fontsize": 11,
          "fontname": "Avenir Next",
          "bgcolor": [
            0.1411764705882353,
            0.1568627450980392,
            0.16470588235294117,
            1.0
          ],
          "activebgcolor": [
            0.1411764705882353,
            0.1568627450980392,
            0.16470588235294117,
            1.0
          ],
          "textcolor": [
            0.9411764705882353,
            0.9490196078431372,
            0.9294117647058824,
            1.0
          ],
          "activetextcolor": [
            0.9411764705882353,
            0.9490196078431372,
            0.9294117647058824,
            1.0
          ],
          "hidden": 0,
          "hint": ""
        }
      },
      {
        "box": {
          "id": "cfg-bars",
          "maxclass": "newobj",
          "patching_rect": [
            20,
            604,
            180,
            22
          ],
          "numinlets": 1,
          "numoutlets": 1,
          "text": "prepend cfg bars"
        }
      },
      {
        "box": {
          "id": "advanced-divider",
          "maxclass": "panel",
          "patching_rect": [
            999,
            10,
            1,
            149
          ],
          "numinlets": 1,
          "numoutlets": 0,
          "presentation": 1,
          "presentation_rect": [
            999,
            10,
            1,
            149
          ],
          "bgcolor": [
            0.2196078431372549,
            0.24313725490196078,
            0.25098039215686274,
            1.0
          ],
          "border": 0,
          "rounded": 0,
          "hidden": 1,
          "ignoreclick": 1,
          "background": 1,
          "varname": "advanced-divider"
        }
      },
      {
        "box": {
          "id": "advanced-title",
          "maxclass": "comment",
          "patching_rect": [
            1008,
            9,
            238,
            19
          ],
          "numinlets": 1,
          "numoutlets": 0,
          "presentation": 1,
          "presentation_rect": [
            1008,
            9,
            238,
            19
          ],
          "text": "Timing & pitch",
          "fontsize": 11,
          "fontname": "Avenir Next",
          "textcolor": [
            0.9411764705882353,
            0.9490196078431372,
            0.9294117647058824,
            1.0
          ],
          "hidden": 1,
          "varname": "advanced-title"
        }
      },
      {
        "box": {
          "id": "label-octave",
          "maxclass": "comment",
          "patching_rect": [
            1008,
            93,
            110,
            17
          ],
          "numinlets": 1,
          "numoutlets": 0,
          "presentation": 1,
          "presentation_rect": [
            1008,
            93,
            110,
            17
          ],
          "text": "Register",
          "fontsize": 11,
          "fontname": "Avenir Next",
          "textcolor": [
            0.6549019607843137,
            0.6862745098039216,
            0.6784313725490196,
            1.0
          ],
          "hidden": 1,
          "varname": "label-octave"
        }
      },
      {
        "box": {
          "id": "octave",
          "maxclass": "live.menu",
          "patching_rect": [
            1008,
            112,
            110,
            22
          ],
          "numinlets": 1,
          "numoutlets": 3,
          "presentation": 1,
          "presentation_rect": [
            1008,
            112,
            110,
            22
          ],
          "varname": "octave",
          "parameter_enable": 1,
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Octave",
              "parameter_shortname": "Octave",
              "parameter_type": 2,
              "parameter_initial_enable": 1,
              "parameter_initial": [
                1
              ],
              "parameter_order": 0,
              "parameter_mmin": 0,
              "parameter_mmax": 2,
              "parameter_enum": [
                "0",
                "1",
                "2"
              ]
            }
          },
          "items": [
            "0",
            "1",
            "2"
          ],
          "fontsize": 11,
          "fontname": "Avenir Next",
          "bgcolor": [
            0.1411764705882353,
            0.1568627450980392,
            0.16470588235294117,
            1.0
          ],
          "activebgcolor": [
            0.1411764705882353,
            0.1568627450980392,
            0.16470588235294117,
            1.0
          ],
          "textcolor": [
            0.9411764705882353,
            0.9490196078431372,
            0.9294117647058824,
            1.0
          ],
          "activetextcolor": [
            0.9411764705882353,
            0.9490196078431372,
            0.9294117647058824,
            1.0
          ],
          "hidden": 1,
          "hint": "Base octave in Live notation. The Key menu sets its pitch class."
        }
      },
      {
        "box": {
          "id": "cfg-octave",
          "maxclass": "newobj",
          "patching_rect": [
            20,
            724,
            180,
            22
          ],
          "numinlets": 1,
          "numoutlets": 1,
          "text": "prepend cfg octave"
        }
      },
      {
        "box": {
          "id": "label-density",
          "maxclass": "comment",
          "patching_rect": [
            16,
            91,
            127,
            18
          ],
          "numinlets": 1,
          "numoutlets": 0,
          "presentation": 1,
          "presentation_rect": [
            16,
            91,
            127,
            18
          ],
          "text": "Density",
          "fontsize": 11,
          "fontname": "Avenir Next",
          "textcolor": [
            0.9411764705882353,
            0.9490196078431372,
            0.9294117647058824,
            1.0
          ],
          "hidden": 0,
          "varname": "label-density"
        }
      },
      {
        "box": {
          "id": "value-density",
          "maxclass": "comment",
          "patching_rect": [
            143,
            91,
            35,
            18
          ],
          "numinlets": 1,
          "numoutlets": 0,
          "presentation": 1,
          "presentation_rect": [
            143,
            91,
            35,
            18
          ],
          "text": "55%",
          "fontsize": 11,
          "fontname": "Avenir Next",
          "textcolor": [
            0.6549019607843137,
            0.6862745098039216,
            0.6784313725490196,
            1.0
          ],
          "justification": 2,
          "hidden": 0,
          "varname": "value-density"
        }
      },
      {
        "box": {
          "id": "density",
          "maxclass": "live.slider",
          "patching_rect": [
            16,
            113,
            162,
            18
          ],
          "numinlets": 1,
          "numoutlets": 2,
          "presentation": 1,
          "presentation_rect": [
            16,
            113,
            162,
            18
          ],
          "varname": "density",
          "parameter_enable": 1,
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Density",
              "parameter_shortname": "Density",
              "parameter_type": 0,
              "parameter_initial_enable": 1,
              "parameter_initial": [
                55
              ],
              "parameter_order": 0,
              "parameter_mmin": 0,
              "parameter_mmax": 100,
              "parameter_unitstyle": 5
            }
          },
          "orientation": 1,
          "showname": 0,
          "shownumber": 0,
          "slidercolor": [
            0.2196078431372549,
            0.24313725490196078,
            0.25098039215686274,
            1.0
          ],
          "tricolor": [
            0.7725490196078432,
            0.9294117647058824,
            0.4627450980392157,
            1.0
          ],
          "trioncolor": [
            0.7725490196078432,
            0.9294117647058824,
            0.4627450980392157,
            1.0
          ],
          "tribordercolor": [
            0.09803921568627451,
            0.10980392156862745,
            0.11764705882352941,
            1.0
          ],
          "focusbordercolor": [
            0.7725490196078432,
            0.9294117647058824,
            0.4627450980392157,
            1.0
          ],
          "textcolor": [
            0.9411764705882353,
            0.9490196078431372,
            0.9294117647058824,
            1.0
          ],
          "hidden": 0,
          "hint": "Fewer or more notes in the next generated phrase."
        }
      },
      {
        "box": {
          "id": "cfg-density",
          "maxclass": "newobj",
          "patching_rect": [
            20,
            820,
            180,
            22
          ],
          "numinlets": 1,
          "numoutlets": 1,
          "text": "prepend cfg density"
        }
      },
      {
        "box": {
          "id": "label-swing",
          "maxclass": "comment",
          "patching_rect": [
            1008,
            40,
            75,
            18
          ],
          "numinlets": 1,
          "numoutlets": 0,
          "presentation": 1,
          "presentation_rect": [
            1008,
            40,
            75,
            18
          ],
          "text": "Swing",
          "fontsize": 11,
          "fontname": "Avenir Next",
          "textcolor": [
            0.9411764705882353,
            0.9490196078431372,
            0.9294117647058824,
            1.0
          ],
          "hidden": 1,
          "varname": "label-swing"
        }
      },
      {
        "box": {
          "id": "value-swing",
          "maxclass": "comment",
          "patching_rect": [
            1083,
            40,
            35,
            18
          ],
          "numinlets": 1,
          "numoutlets": 0,
          "presentation": 1,
          "presentation_rect": [
            1083,
            40,
            35,
            18
          ],
          "text": "12%",
          "fontsize": 11,
          "fontname": "Avenir Next",
          "textcolor": [
            0.6549019607843137,
            0.6862745098039216,
            0.6784313725490196,
            1.0
          ],
          "justification": 2,
          "hidden": 1,
          "varname": "value-swing"
        }
      },
      {
        "box": {
          "id": "swing",
          "maxclass": "live.slider",
          "patching_rect": [
            1008,
            62,
            110,
            18
          ],
          "numinlets": 1,
          "numoutlets": 2,
          "presentation": 1,
          "presentation_rect": [
            1008,
            62,
            110,
            18
          ],
          "varname": "swing",
          "parameter_enable": 1,
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Swing",
              "parameter_shortname": "Swing",
              "parameter_type": 0,
              "parameter_initial_enable": 1,
              "parameter_initial": [
                12
              ],
              "parameter_order": 0,
              "parameter_mmin": 0,
              "parameter_mmax": 100,
              "parameter_unitstyle": 5
            }
          },
          "orientation": 1,
          "showname": 0,
          "shownumber": 0,
          "slidercolor": [
            0.2196078431372549,
            0.24313725490196078,
            0.25098039215686274,
            1.0
          ],
          "tricolor": [
            0.7725490196078432,
            0.9294117647058824,
            0.4627450980392157,
            1.0
          ],
          "trioncolor": [
            0.7725490196078432,
            0.9294117647058824,
            0.4627450980392157,
            1.0
          ],
          "tribordercolor": [
            0.09803921568627451,
            0.10980392156862745,
            0.11764705882352941,
            1.0
          ],
          "focusbordercolor": [
            0.7725490196078432,
            0.9294117647058824,
            0.4627450980392157,
            1.0
          ],
          "textcolor": [
            0.9411764705882353,
            0.9490196078431372,
            0.9294117647058824,
            1.0
          ],
          "hidden": 1,
          "hint": "Delay alternate sixteenth notes. Generate to apply."
        }
      },
      {
        "box": {
          "id": "cfg-swing",
          "maxclass": "newobj",
          "patching_rect": [
            20,
            916,
            180,
            22
          ],
          "numinlets": 1,
          "numoutlets": 1,
          "text": "prepend cfg swing"
        }
      },
      {
        "box": {
          "id": "label-gate",
          "maxclass": "comment",
          "patching_rect": [
            1134,
            40,
            75,
            18
          ],
          "numinlets": 1,
          "numoutlets": 0,
          "presentation": 1,
          "presentation_rect": [
            1134,
            40,
            75,
            18
          ],
          "text": "Note length",
          "fontsize": 11,
          "fontname": "Avenir Next",
          "textcolor": [
            0.9411764705882353,
            0.9490196078431372,
            0.9294117647058824,
            1.0
          ],
          "hidden": 1,
          "varname": "label-gate"
        }
      },
      {
        "box": {
          "id": "value-gate",
          "maxclass": "comment",
          "patching_rect": [
            1209,
            40,
            35,
            18
          ],
          "numinlets": 1,
          "numoutlets": 0,
          "presentation": 1,
          "presentation_rect": [
            1209,
            40,
            35,
            18
          ],
          "text": "65%",
          "fontsize": 11,
          "fontname": "Avenir Next",
          "textcolor": [
            0.6549019607843137,
            0.6862745098039216,
            0.6784313725490196,
            1.0
          ],
          "justification": 2,
          "hidden": 1,
          "varname": "value-gate"
        }
      },
      {
        "box": {
          "id": "gate",
          "maxclass": "live.slider",
          "patching_rect": [
            1134,
            62,
            110,
            18
          ],
          "numinlets": 1,
          "numoutlets": 2,
          "presentation": 1,
          "presentation_rect": [
            1134,
            62,
            110,
            18
          ],
          "varname": "gate",
          "parameter_enable": 1,
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Gate",
              "parameter_shortname": "Gate",
              "parameter_type": 0,
              "parameter_initial_enable": 1,
              "parameter_initial": [
                65
              ],
              "parameter_order": 0,
              "parameter_mmin": 0,
              "parameter_mmax": 100,
              "parameter_unitstyle": 5
            }
          },
          "orientation": 1,
          "showname": 0,
          "shownumber": 0,
          "slidercolor": [
            0.2196078431372549,
            0.24313725490196078,
            0.25098039215686274,
            1.0
          ],
          "tricolor": [
            0.7725490196078432,
            0.9294117647058824,
            0.4627450980392157,
            1.0
          ],
          "trioncolor": [
            0.7725490196078432,
            0.9294117647058824,
            0.4627450980392157,
            1.0
          ],
          "tribordercolor": [
            0.09803921568627451,
            0.10980392156862745,
            0.11764705882352941,
            1.0
          ],
          "focusbordercolor": [
            0.7725490196078432,
            0.9294117647058824,
            0.4627450980392157,
            1.0
          ],
          "textcolor": [
            0.9411764705882353,
            0.9490196078431372,
            0.9294117647058824,
            1.0
          ],
          "hidden": 1,
          "hint": "Shorter or longer notes. Generate to apply."
        }
      },
      {
        "box": {
          "id": "cfg-gate",
          "maxclass": "newobj",
          "patching_rect": [
            20,
            1012,
            180,
            22
          ],
          "numinlets": 1,
          "numoutlets": 1,
          "text": "prepend cfg gate"
        }
      },
      {
        "box": {
          "id": "label-movement",
          "maxclass": "comment",
          "patching_rect": [
            198,
            91,
            127,
            18
          ],
          "numinlets": 1,
          "numoutlets": 0,
          "presentation": 1,
          "presentation_rect": [
            198,
            91,
            127,
            18
          ],
          "text": "Melody",
          "fontsize": 11,
          "fontname": "Avenir Next",
          "textcolor": [
            0.9411764705882353,
            0.9490196078431372,
            0.9294117647058824,
            1.0
          ],
          "hidden": 0,
          "varname": "label-movement"
        }
      },
      {
        "box": {
          "id": "value-movement",
          "maxclass": "comment",
          "patching_rect": [
            325,
            91,
            35,
            18
          ],
          "numinlets": 1,
          "numoutlets": 0,
          "presentation": 1,
          "presentation_rect": [
            325,
            91,
            35,
            18
          ],
          "text": "35%",
          "fontsize": 11,
          "fontname": "Avenir Next",
          "textcolor": [
            0.6549019607843137,
            0.6862745098039216,
            0.6784313725490196,
            1.0
          ],
          "justification": 2,
          "hidden": 0,
          "varname": "value-movement"
        }
      },
      {
        "box": {
          "id": "movement",
          "maxclass": "live.slider",
          "patching_rect": [
            198,
            113,
            162,
            18
          ],
          "numinlets": 1,
          "numoutlets": 2,
          "presentation": 1,
          "presentation_rect": [
            198,
            113,
            162,
            18
          ],
          "varname": "movement",
          "parameter_enable": 1,
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Movement",
              "parameter_shortname": "Movement",
              "parameter_type": 0,
              "parameter_initial_enable": 1,
              "parameter_initial": [
                35
              ],
              "parameter_order": 0,
              "parameter_mmin": 0,
              "parameter_mmax": 100,
              "parameter_unitstyle": 5
            }
          },
          "orientation": 1,
          "showname": 0,
          "shownumber": 0,
          "slidercolor": [
            0.2196078431372549,
            0.24313725490196078,
            0.25098039215686274,
            1.0
          ],
          "tricolor": [
            0.7725490196078432,
            0.9294117647058824,
            0.4627450980392157,
            1.0
          ],
          "trioncolor": [
            0.7725490196078432,
            0.9294117647058824,
            0.4627450980392157,
            1.0
          ],
          "tribordercolor": [
            0.09803921568627451,
            0.10980392156862745,
            0.11764705882352941,
            1.0
          ],
          "focusbordercolor": [
            0.7725490196078432,
            0.9294117647058824,
            0.4627450980392157,
            1.0
          ],
          "textcolor": [
            0.9411764705882353,
            0.9490196078431372,
            0.9294117647058824,
            1.0
          ],
          "hidden": 0,
          "hint": "Less or more pitch movement in the next generated phrase."
        }
      },
      {
        "box": {
          "id": "cfg-movement",
          "maxclass": "newobj",
          "patching_rect": [
            20,
            1108,
            180,
            22
          ],
          "numinlets": 1,
          "numoutlets": 1,
          "text": "prepend cfg movement"
        }
      },
      {
        "box": {
          "id": "rootOnly",
          "maxclass": "live.text",
          "patching_rect": [
            1134,
            90,
            110,
            20
          ],
          "numinlets": 1,
          "numoutlets": 2,
          "presentation": 1,
          "presentation_rect": [
            1134,
            90,
            110,
            20
          ],
          "varname": "rootOnly",
          "mode": 1,
          "text": "Root notes only",
          "texton": "Root notes only",
          "parameter_enable": 1,
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "ROOT ONLY",
              "parameter_shortname": "ROOT ONLY",
              "parameter_type": 2,
              "parameter_initial_enable": 1,
              "parameter_initial": [
                0
              ],
              "parameter_order": 0,
              "parameter_mmin": 0,
              "parameter_mmax": 1,
              "parameter_enum": [
                "Off",
                "On"
              ]
            }
          },
          "fontsize": 10,
          "fontname": "Avenir Next",
          "activebgcolor": [
            0.1411764705882353,
            0.1568627450980392,
            0.16470588235294117,
            1.0
          ],
          "activebgoncolor": [
            0.7725490196078432,
            0.9294117647058824,
            0.4627450980392157,
            1.0
          ],
          "activetextcolor": [
            0.6549019607843137,
            0.6862745098039216,
            0.6784313725490196,
            1.0
          ],
          "activetextoncolor": [
            0.09803921568627451,
            0.10980392156862745,
            0.11764705882352941,
            1.0
          ],
          "rounded": 3,
          "hidden": 1,
          "hint": "Use only the selected key's root pitch class for new notes."
        }
      },
      {
        "box": {
          "id": "cfg-rootOnly",
          "maxclass": "newobj",
          "patching_rect": [
            20,
            1156,
            180,
            22
          ],
          "numinlets": 1,
          "numoutlets": 1,
          "text": "prepend cfg rootOnly"
        }
      },
      {
        "box": {
          "id": "anchorRoot",
          "maxclass": "live.text",
          "patching_rect": [
            1134,
            115,
            110,
            20
          ],
          "numinlets": 1,
          "numoutlets": 2,
          "presentation": 1,
          "presentation_rect": [
            1134,
            115,
            110,
            20
          ],
          "varname": "anchorRoot",
          "mode": 1,
          "text": "Start on root",
          "texton": "Start on root",
          "parameter_enable": 1,
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "ROOT ANCHOR",
              "parameter_shortname": "ROOT ANCHOR",
              "parameter_type": 2,
              "parameter_initial_enable": 1,
              "parameter_initial": [
                1
              ],
              "parameter_order": 0,
              "parameter_mmin": 0,
              "parameter_mmax": 1,
              "parameter_enum": [
                "Off",
                "On"
              ]
            }
          },
          "fontsize": 10,
          "fontname": "Avenir Next",
          "activebgcolor": [
            0.1411764705882353,
            0.1568627450980392,
            0.16470588235294117,
            1.0
          ],
          "activebgoncolor": [
            0.7725490196078432,
            0.9294117647058824,
            0.4627450980392157,
            1.0
          ],
          "activetextcolor": [
            0.6549019607843137,
            0.6862745098039216,
            0.6784313725490196,
            1.0
          ],
          "activetextoncolor": [
            0.09803921568627451,
            0.10980392156862745,
            0.11764705882352941,
            1.0
          ],
          "rounded": 3,
          "hidden": 1,
          "hint": "Start on the root note without moving its rhythmic position."
        }
      },
      {
        "box": {
          "id": "cfg-anchorRoot",
          "maxclass": "newobj",
          "patching_rect": [
            20,
            1204,
            180,
            22
          ],
          "numinlets": 1,
          "numoutlets": 1,
          "text": "prepend cfg anchorRoot"
        }
      },
      {
        "box": {
          "id": "octaveJumps",
          "maxclass": "live.text",
          "patching_rect": [
            1134,
            140,
            110,
            20
          ],
          "numinlets": 1,
          "numoutlets": 2,
          "presentation": 1,
          "presentation_rect": [
            1134,
            140,
            110,
            20
          ],
          "varname": "octaveJumps",
          "mode": 1,
          "text": "Octave jumps",
          "texton": "Octave jumps",
          "parameter_enable": 1,
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "OCTAVE JUMPS",
              "parameter_shortname": "OCTAVE JUMPS",
              "parameter_type": 2,
              "parameter_initial_enable": 1,
              "parameter_initial": [
                1
              ],
              "parameter_order": 0,
              "parameter_mmin": 0,
              "parameter_mmax": 1,
              "parameter_enum": [
                "Off",
                "On"
              ]
            }
          },
          "fontsize": 10,
          "fontname": "Avenir Next",
          "activebgcolor": [
            0.1411764705882353,
            0.1568627450980392,
            0.16470588235294117,
            1.0
          ],
          "activebgoncolor": [
            0.7725490196078432,
            0.9294117647058824,
            0.4627450980392157,
            1.0
          ],
          "activetextcolor": [
            0.6549019607843137,
            0.6862745098039216,
            0.6784313725490196,
            1.0
          ],
          "activetextoncolor": [
            0.09803921568627451,
            0.10980392156862745,
            0.11764705882352941,
            1.0
          ],
          "rounded": 3,
          "hidden": 1,
          "hint": "Allow octave changes while generating new notes."
        }
      },
      {
        "box": {
          "id": "cfg-octaveJumps",
          "maxclass": "newobj",
          "patching_rect": [
            20,
            1252,
            180,
            22
          ],
          "numinlets": 1,
          "numoutlets": 1,
          "text": "prepend cfg octaveJumps"
        }
      },
      {
        "box": {
          "id": "roll",
          "maxclass": "jsui",
          "patching_rect": [
            380,
            40,
            416,
            100
          ],
          "numinlets": 1,
          "numoutlets": 1,
          "presentation": 1,
          "presentation_rect": [
            380,
            40,
            416,
            100
          ],
          "filename": "undertow-roll.js",
          "jsarguments": [],
          "parameter_enable": 0,
          "varname": "roll",
          "border": 0,
          "hint": "Click a note to lock or unlock it. Locked notes stay exact when generating or changing a few notes."
        }
      },
      {
        "box": {
          "id": "generate",
          "maxclass": "live.text",
          "patching_rect": [
            816,
            40,
            168,
            24
          ],
          "numinlets": 1,
          "numoutlets": 2,
          "presentation": 1,
          "presentation_rect": [
            816,
            40,
            168,
            24
          ],
          "varname": "action_generate",
          "mode": 0,
          "transition": 0,
          "text": "Generate",
          "texton": "Generate",
          "parameter_enable": 1,
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "GENERATE",
              "parameter_shortname": "GENERATE",
              "parameter_type": 2,
              "parameter_initial_enable": 0,
              "parameter_initial": [
                0
              ],
              "parameter_order": 0,
              "parameter_mmin": 0,
              "parameter_mmax": 1,
              "parameter_enum": [
                "Idle",
                "Trigger"
              ],
              "parameter_invisible": 4,
              "parameter_visible_undoable": 0
            }
          },
          "fontsize": 11,
          "fontname": "Avenir Next",
          "activebgcolor": [
            0.7725490196078432,
            0.9294117647058824,
            0.4627450980392157,
            1.0
          ],
          "activebgoncolor": [
            0.7725490196078432,
            0.9294117647058824,
            0.4627450980392157,
            1.0
          ],
          "activetextcolor": [
            0.09803921568627451,
            0.10980392156862745,
            0.11764705882352941,
            1.0
          ],
          "activetextoncolor": [
            0.09803921568627451,
            0.10980392156862745,
            0.11764705882352941,
            1.0
          ],
          "bgcolor": [
            0.09803921568627451,
            0.10980392156862745,
            0.11764705882352941,
            1.0
          ],
          "textcolor": [
            0.4117647058823529,
            0.44313725490196076,
            0.43529411764705883,
            1.0
          ],
          "bordercolor": [
            0.2196078431372549,
            0.24313725490196078,
            0.25098039215686274,
            1.0
          ],
          "rounded": 3,
          "hidden": 0,
          "hint": "Generate a new bassline using the selected settings. Locked notes stay exact."
        }
      },
      {
        "box": {
          "id": "action-generate",
          "maxclass": "message",
          "patching_rect": [
            930,
            1220,
            110,
            22
          ],
          "numinlets": 1,
          "numoutlets": 1,
          "text": "generate"
        }
      },
      {
        "box": {
          "id": "vary",
          "maxclass": "live.text",
          "patching_rect": [
            816,
            69,
            168,
            22
          ],
          "numinlets": 1,
          "numoutlets": 2,
          "presentation": 1,
          "presentation_rect": [
            816,
            69,
            168,
            22
          ],
          "varname": "action_vary",
          "mode": 0,
          "transition": 0,
          "text": "Change a few notes",
          "texton": "Change a few notes",
          "parameter_enable": 1,
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Variation",
              "parameter_shortname": "Variation",
              "parameter_type": 2,
              "parameter_initial_enable": 0,
              "parameter_initial": [
                0
              ],
              "parameter_order": 0,
              "parameter_mmin": 0,
              "parameter_mmax": 1,
              "parameter_enum": [
                "Idle",
                "Trigger"
              ],
              "parameter_invisible": 4,
              "parameter_visible_undoable": 0
            }
          },
          "fontsize": 11,
          "fontname": "Avenir Next",
          "activebgcolor": [
            0.1411764705882353,
            0.1568627450980392,
            0.16470588235294117,
            1.0
          ],
          "activebgoncolor": [
            0.7725490196078432,
            0.9294117647058824,
            0.4627450980392157,
            1.0
          ],
          "activetextcolor": [
            0.9411764705882353,
            0.9490196078431372,
            0.9294117647058824,
            1.0
          ],
          "activetextoncolor": [
            0.09803921568627451,
            0.10980392156862745,
            0.11764705882352941,
            1.0
          ],
          "bgcolor": [
            0.09803921568627451,
            0.10980392156862745,
            0.11764705882352941,
            1.0
          ],
          "textcolor": [
            0.4117647058823529,
            0.44313725490196076,
            0.43529411764705883,
            1.0
          ],
          "bordercolor": [
            0.2196078431372549,
            0.24313725490196078,
            0.25098039215686274,
            1.0
          ],
          "rounded": 3,
          "hidden": 0,
          "hint": "Keep this rhythm and most notes; change a few unlocked notes. Apply changed settings with Generate first."
        }
      },
      {
        "box": {
          "id": "action-vary",
          "maxclass": "message",
          "patching_rect": [
            930,
            1260,
            110,
            22
          ],
          "numinlets": 1,
          "numoutlets": 1,
          "text": "vary"
        }
      },
      {
        "box": {
          "id": "undo",
          "maxclass": "live.text",
          "patching_rect": [
            704,
            9,
            42,
            20
          ],
          "numinlets": 1,
          "numoutlets": 2,
          "presentation": 1,
          "presentation_rect": [
            704,
            9,
            42,
            20
          ],
          "varname": "action_undo",
          "mode": 0,
          "transition": 0,
          "text": "Undo",
          "texton": "Undo",
          "parameter_enable": 1,
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Undo",
              "parameter_shortname": "Undo",
              "parameter_type": 2,
              "parameter_initial_enable": 0,
              "parameter_initial": [
                0
              ],
              "parameter_order": 0,
              "parameter_mmin": 0,
              "parameter_mmax": 1,
              "parameter_enum": [
                "Idle",
                "Trigger"
              ],
              "parameter_invisible": 4,
              "parameter_visible_undoable": 0
            }
          },
          "fontsize": 11,
          "fontname": "Avenir Next",
          "activebgcolor": [
            0.1411764705882353,
            0.1568627450980392,
            0.16470588235294117,
            1.0
          ],
          "activebgoncolor": [
            0.7725490196078432,
            0.9294117647058824,
            0.4627450980392157,
            1.0
          ],
          "activetextcolor": [
            0.9411764705882353,
            0.9490196078431372,
            0.9294117647058824,
            1.0
          ],
          "activetextoncolor": [
            0.09803921568627451,
            0.10980392156862745,
            0.11764705882352941,
            1.0
          ],
          "bgcolor": [
            0.09803921568627451,
            0.10980392156862745,
            0.11764705882352941,
            1.0
          ],
          "textcolor": [
            0.4117647058823529,
            0.44313725490196076,
            0.43529411764705883,
            1.0
          ],
          "bordercolor": [
            0.2196078431372549,
            0.24313725490196078,
            0.25098039215686274,
            1.0
          ],
          "rounded": 3,
          "hidden": 0,
          "hint": "Restore the previous phrase or note locks."
        }
      },
      {
        "box": {
          "id": "action-undo",
          "maxclass": "message",
          "patching_rect": [
            930,
            1300,
            110,
            22
          ],
          "numinlets": 1,
          "numoutlets": 1,
          "text": "undo"
        }
      },
      {
        "box": {
          "id": "redo",
          "maxclass": "live.text",
          "patching_rect": [
            752,
            9,
            44,
            20
          ],
          "numinlets": 1,
          "numoutlets": 2,
          "presentation": 1,
          "presentation_rect": [
            752,
            9,
            44,
            20
          ],
          "varname": "action_redo",
          "mode": 0,
          "transition": 0,
          "text": "Redo",
          "texton": "Redo",
          "parameter_enable": 1,
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Redo",
              "parameter_shortname": "Redo",
              "parameter_type": 2,
              "parameter_initial_enable": 0,
              "parameter_initial": [
                0
              ],
              "parameter_order": 0,
              "parameter_mmin": 0,
              "parameter_mmax": 1,
              "parameter_enum": [
                "Idle",
                "Trigger"
              ],
              "parameter_invisible": 4,
              "parameter_visible_undoable": 0
            }
          },
          "fontsize": 11,
          "fontname": "Avenir Next",
          "activebgcolor": [
            0.1411764705882353,
            0.1568627450980392,
            0.16470588235294117,
            1.0
          ],
          "activebgoncolor": [
            0.7725490196078432,
            0.9294117647058824,
            0.4627450980392157,
            1.0
          ],
          "activetextcolor": [
            0.9411764705882353,
            0.9490196078431372,
            0.9294117647058824,
            1.0
          ],
          "activetextoncolor": [
            0.09803921568627451,
            0.10980392156862745,
            0.11764705882352941,
            1.0
          ],
          "bgcolor": [
            0.09803921568627451,
            0.10980392156862745,
            0.11764705882352941,
            1.0
          ],
          "textcolor": [
            0.4117647058823529,
            0.44313725490196076,
            0.43529411764705883,
            1.0
          ],
          "bordercolor": [
            0.2196078431372549,
            0.24313725490196078,
            0.25098039215686274,
            1.0
          ],
          "rounded": 3,
          "hidden": 0,
          "hint": "Restore the next phrase or note locks."
        }
      },
      {
        "box": {
          "id": "action-redo",
          "maxclass": "message",
          "patching_rect": [
            930,
            1340,
            110,
            22
          ],
          "numinlets": 1,
          "numoutlets": 1,
          "text": "redo"
        }
      },
      {
        "box": {
          "id": "clip",
          "maxclass": "live.text",
          "patching_rect": [
            816,
            105,
            82,
            22
          ],
          "numinlets": 1,
          "numoutlets": 2,
          "presentation": 1,
          "presentation_rect": [
            816,
            105,
            82,
            22
          ],
          "varname": "action_clip",
          "mode": 0,
          "transition": 0,
          "text": "Create clip",
          "texton": "Create clip",
          "parameter_enable": 1,
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "CREATE CLIP",
              "parameter_shortname": "CREATE CLIP",
              "parameter_type": 2,
              "parameter_initial_enable": 0,
              "parameter_initial": [
                0
              ],
              "parameter_order": 0,
              "parameter_mmin": 0,
              "parameter_mmax": 1,
              "parameter_enum": [
                "Idle",
                "Trigger"
              ],
              "parameter_invisible": 4,
              "parameter_visible_undoable": 0
            }
          },
          "fontsize": 11,
          "fontname": "Avenir Next",
          "activebgcolor": [
            0.1411764705882353,
            0.1568627450980392,
            0.16470588235294117,
            1.0
          ],
          "activebgoncolor": [
            0.7725490196078432,
            0.9294117647058824,
            0.4627450980392157,
            1.0
          ],
          "activetextcolor": [
            0.9411764705882353,
            0.9490196078431372,
            0.9294117647058824,
            1.0
          ],
          "activetextoncolor": [
            0.09803921568627451,
            0.10980392156862745,
            0.11764705882352941,
            1.0
          ],
          "bgcolor": [
            0.09803921568627451,
            0.10980392156862745,
            0.11764705882352941,
            1.0
          ],
          "textcolor": [
            0.4117647058823529,
            0.44313725490196076,
            0.43529411764705883,
            1.0
          ],
          "bordercolor": [
            0.2196078431372549,
            0.24313725490196078,
            0.25098039215686274,
            1.0
          ],
          "rounded": 3,
          "hidden": 0,
          "hint": "Create a clip in this track's empty slot in the selected Session scene."
        }
      },
      {
        "box": {
          "id": "action-clip",
          "maxclass": "message",
          "patching_rect": [
            930,
            1380,
            110,
            22
          ],
          "numinlets": 1,
          "numoutlets": 1,
          "text": "createclip"
        }
      },
      {
        "box": {
          "id": "export",
          "maxclass": "live.text",
          "patching_rect": [
            904,
            105,
            80,
            22
          ],
          "numinlets": 1,
          "numoutlets": 2,
          "presentation": 1,
          "presentation_rect": [
            904,
            105,
            80,
            22
          ],
          "varname": "action_export",
          "mode": 0,
          "transition": 0,
          "text": "Save MIDI",
          "texton": "Save MIDI",
          "parameter_enable": 1,
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "SAVE MIDI",
              "parameter_shortname": "SAVE MIDI",
              "parameter_type": 2,
              "parameter_initial_enable": 0,
              "parameter_initial": [
                0
              ],
              "parameter_order": 0,
              "parameter_mmin": 0,
              "parameter_mmax": 1,
              "parameter_enum": [
                "Idle",
                "Trigger"
              ],
              "parameter_invisible": 4,
              "parameter_visible_undoable": 0
            }
          },
          "fontsize": 11,
          "fontname": "Avenir Next",
          "activebgcolor": [
            0.1411764705882353,
            0.1568627450980392,
            0.16470588235294117,
            1.0
          ],
          "activebgoncolor": [
            0.7725490196078432,
            0.9294117647058824,
            0.4627450980392157,
            1.0
          ],
          "activetextcolor": [
            0.9411764705882353,
            0.9490196078431372,
            0.9294117647058824,
            1.0
          ],
          "activetextoncolor": [
            0.09803921568627451,
            0.10980392156862745,
            0.11764705882352941,
            1.0
          ],
          "bgcolor": [
            0.09803921568627451,
            0.10980392156862745,
            0.11764705882352941,
            1.0
          ],
          "textcolor": [
            0.4117647058823529,
            0.44313725490196076,
            0.43529411764705883,
            1.0
          ],
          "bordercolor": [
            0.2196078431372549,
            0.24313725490196078,
            0.25098039215686274,
            1.0
          ],
          "rounded": 3,
          "hidden": 0,
          "hint": "Save the displayed notes as a MIDI file."
        }
      },
      {
        "box": {
          "id": "action-export",
          "maxclass": "message",
          "patching_rect": [
            930,
            1420,
            110,
            22
          ],
          "numinlets": 1,
          "numoutlets": 1,
          "text": "beginexport"
        }
      },
      {
        "box": {
          "id": "output",
          "maxclass": "live.text",
          "patching_rect": [
            888,
            9,
            96,
            20
          ],
          "numinlets": 1,
          "numoutlets": 2,
          "presentation": 1,
          "presentation_rect": [
            888,
            9,
            96,
            20
          ],
          "varname": "output",
          "mode": 1,
          "text": "Live MIDI off",
          "texton": "Live MIDI on",
          "parameter_enable": 1,
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "MIDI ON",
              "parameter_shortname": "MIDI ON",
              "parameter_type": 2,
              "parameter_initial_enable": 1,
              "parameter_initial": [
                1
              ],
              "parameter_order": 0,
              "parameter_mmin": 0,
              "parameter_mmax": 1,
              "parameter_enum": [
                "Off",
                "On"
              ]
            }
          },
          "fontsize": 10,
          "fontname": "Avenir Next",
          "activebgcolor": [
            0.1411764705882353,
            0.1568627450980392,
            0.16470588235294117,
            1.0
          ],
          "activebgoncolor": [
            0.7725490196078432,
            0.9294117647058824,
            0.4627450980392157,
            1.0
          ],
          "activetextcolor": [
            0.6549019607843137,
            0.6862745098039216,
            0.6784313725490196,
            1.0
          ],
          "activetextoncolor": [
            0.09803921568627451,
            0.10980392156862745,
            0.11764705882352941,
            1.0
          ],
          "rounded": 3,
          "hidden": 0,
          "hint": "Play generated notes through the next instrument when Live is playing. Off passes track MIDI through."
        }
      },
      {
        "box": {
          "id": "cfg-output",
          "maxclass": "newobj",
          "patching_rect": [
            20,
            1612,
            180,
            22
          ],
          "numinlets": 1,
          "numoutlets": 1,
          "text": "prepend output"
        }
      },
      {
        "box": {
          "id": "panic",
          "maxclass": "live.text",
          "patching_rect": [
            1008,
            140,
            110,
            20
          ],
          "numinlets": 1,
          "numoutlets": 2,
          "presentation": 1,
          "presentation_rect": [
            1008,
            140,
            110,
            20
          ],
          "varname": "action_panic",
          "mode": 0,
          "transition": 0,
          "text": "Stop notes",
          "texton": "Stop notes",
          "parameter_enable": 1,
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Panic",
              "parameter_shortname": "Panic",
              "parameter_type": 2,
              "parameter_initial_enable": 0,
              "parameter_initial": [
                0
              ],
              "parameter_order": 0,
              "parameter_mmin": 0,
              "parameter_mmax": 1,
              "parameter_enum": [
                "Idle",
                "Trigger"
              ],
              "parameter_invisible": 4,
              "parameter_visible_undoable": 0
            }
          },
          "fontsize": 11,
          "fontname": "Avenir Next",
          "activebgcolor": [
            0.1411764705882353,
            0.1568627450980392,
            0.16470588235294117,
            1.0
          ],
          "activebgoncolor": [
            0.7725490196078432,
            0.9294117647058824,
            0.4627450980392157,
            1.0
          ],
          "activetextcolor": [
            0.9411764705882353,
            0.9490196078431372,
            0.9294117647058824,
            1.0
          ],
          "activetextoncolor": [
            0.09803921568627451,
            0.10980392156862745,
            0.11764705882352941,
            1.0
          ],
          "bgcolor": [
            0.09803921568627451,
            0.10980392156862745,
            0.11764705882352941,
            1.0
          ],
          "textcolor": [
            0.4117647058823529,
            0.44313725490196076,
            0.43529411764705883,
            1.0
          ],
          "bordercolor": [
            0.2196078431372549,
            0.24313725490196078,
            0.25098039215686274,
            1.0
          ],
          "rounded": 3,
          "hidden": 1,
          "hint": ""
        }
      },
      {
        "box": {
          "id": "action-panic",
          "maxclass": "message",
          "patching_rect": [
            930,
            1500,
            110,
            22
          ],
          "numinlets": 1,
          "numoutlets": 1,
          "text": "panic"
        }
      },
      {
        "box": {
          "id": "unlock",
          "maxclass": "live.text",
          "patching_rect": [
            614,
            9,
            84,
            20
          ],
          "numinlets": 1,
          "numoutlets": 2,
          "presentation": 1,
          "presentation_rect": [
            614,
            9,
            84,
            20
          ],
          "varname": "action_unlock",
          "mode": 0,
          "transition": 0,
          "text": "Unlock all",
          "texton": "Unlock all",
          "parameter_enable": 1,
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Unlock notes",
              "parameter_shortname": "Unlock notes",
              "parameter_type": 2,
              "parameter_initial_enable": 0,
              "parameter_initial": [
                0
              ],
              "parameter_order": 0,
              "parameter_mmin": 0,
              "parameter_mmax": 1,
              "parameter_enum": [
                "Idle",
                "Trigger"
              ],
              "parameter_invisible": 4,
              "parameter_visible_undoable": 0
            }
          },
          "fontsize": 11,
          "fontname": "Avenir Next",
          "activebgcolor": [
            0.1411764705882353,
            0.1568627450980392,
            0.16470588235294117,
            1.0
          ],
          "activebgoncolor": [
            0.7725490196078432,
            0.9294117647058824,
            0.4627450980392157,
            1.0
          ],
          "activetextcolor": [
            0.9411764705882353,
            0.9490196078431372,
            0.9294117647058824,
            1.0
          ],
          "activetextoncolor": [
            0.09803921568627451,
            0.10980392156862745,
            0.11764705882352941,
            1.0
          ],
          "bgcolor": [
            0.09803921568627451,
            0.10980392156862745,
            0.11764705882352941,
            1.0
          ],
          "textcolor": [
            0.4117647058823529,
            0.44313725490196076,
            0.43529411764705883,
            1.0
          ],
          "bordercolor": [
            0.2196078431372549,
            0.24313725490196078,
            0.25098039215686274,
            1.0
          ],
          "rounded": 3,
          "hidden": 1,
          "hint": "Release every note lock; the current notes stay unchanged."
        }
      },
      {
        "box": {
          "id": "action-unlock",
          "maxclass": "message",
          "patching_rect": [
            930,
            1540,
            110,
            22
          ],
          "numinlets": 1,
          "numoutlets": 1,
          "text": "unlock"
        }
      },
      {
        "box": {
          "id": "more",
          "maxclass": "live.text",
          "patching_rect": [
            864,
            142,
            120,
            20
          ],
          "numinlets": 1,
          "numoutlets": 2,
          "presentation": 1,
          "presentation_rect": [
            864,
            142,
            120,
            20
          ],
          "varname": "action_more",
          "mode": 0,
          "transition": 0,
          "text": "More controls",
          "texton": "More controls",
          "parameter_enable": 1,
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "More controls",
              "parameter_shortname": "More controls",
              "parameter_type": 2,
              "parameter_initial_enable": 0,
              "parameter_initial": [
                0
              ],
              "parameter_order": 0,
              "parameter_mmin": 0,
              "parameter_mmax": 1,
              "parameter_enum": [
                "Idle",
                "Trigger"
              ],
              "parameter_invisible": 4,
              "parameter_visible_undoable": 0
            }
          },
          "fontsize": 11,
          "fontname": "Avenir Next",
          "activebgcolor": [
            0.1411764705882353,
            0.1568627450980392,
            0.16470588235294117,
            1.0
          ],
          "activebgoncolor": [
            0.7725490196078432,
            0.9294117647058824,
            0.4627450980392157,
            1.0
          ],
          "activetextcolor": [
            0.9411764705882353,
            0.9490196078431372,
            0.9294117647058824,
            1.0
          ],
          "activetextoncolor": [
            0.09803921568627451,
            0.10980392156862745,
            0.11764705882352941,
            1.0
          ],
          "bgcolor": [
            0.09803921568627451,
            0.10980392156862745,
            0.11764705882352941,
            1.0
          ],
          "textcolor": [
            0.4117647058823529,
            0.44313725490196076,
            0.43529411764705883,
            1.0
          ],
          "bordercolor": [
            0.2196078431372549,
            0.24313725490196078,
            0.25098039215686274,
            1.0
          ],
          "rounded": 3,
          "hidden": 0,
          "hint": "Show or hide timing and pitch controls."
        }
      },
      {
        "box": {
          "id": "action-more",
          "maxclass": "message",
          "patching_rect": [
            930,
            1580,
            110,
            22
          ],
          "numinlets": 1,
          "numoutlets": 1,
          "text": "details"
        }
      },
      {
        "box": {
          "id": "engine",
          "maxclass": "newobj",
          "patching_rect": [
            448,
            250,
            260,
            23
          ],
          "numinlets": 1,
          "numoutlets": 4,
          "text": "js undertow.js",
          "varname": "engine"
        }
      },
      {
        "box": {
          "id": "midi-in",
          "maxclass": "newobj",
          "patching_rect": [
            20,
            250,
            60,
            22
          ],
          "numinlets": 1,
          "numoutlets": 1,
          "text": "midiin"
        }
      },
      {
        "box": {
          "id": "thru",
          "maxclass": "newobj",
          "patching_rect": [
            20,
            290,
            65,
            22
          ],
          "numinlets": 2,
          "numoutlets": 1,
          "text": "gate 1 0"
        }
      },
      {
        "box": {
          "id": "midi-out",
          "maxclass": "newobj",
          "patching_rect": [
            20,
            340,
            60,
            22
          ],
          "numinlets": 1,
          "numoutlets": 0,
          "text": "midiout"
        }
      },
      {
        "box": {
          "id": "clock",
          "maxclass": "newobj",
          "patching_rect": [
            150,
            250,
            150,
            22
          ],
          "numinlets": 1,
          "numoutlets": 1,
          "text": "metro 2 @active 1"
        }
      },
      {
        "box": {
          "id": "transport-reader",
          "maxclass": "newobj",
          "patching_rect": [
            150,
            290,
            190,
            22
          ],
          "numinlets": 2,
          "numoutlets": 9,
          "text": "transport"
        }
      },
      {
        "box": {
          "id": "position-message",
          "maxclass": "newobj",
          "patching_rect": [
            180,
            330,
            130,
            22
          ],
          "numinlets": 1,
          "numoutlets": 1,
          "text": "prepend position"
        }
      },
      {
        "box": {
          "id": "playing-message",
          "maxclass": "newobj",
          "patching_rect": [
            180,
            360,
            130,
            22
          ],
          "numinlets": 1,
          "numoutlets": 1,
          "text": "prepend playing"
        }
      },
      {
        "box": {
          "id": "pulse-message",
          "maxclass": "message",
          "patching_rect": [
            180,
            390,
            70,
            22
          ],
          "numinlets": 1,
          "numoutlets": 1,
          "text": "pulse"
        }
      },
      {
        "box": {
          "id": "this-device",
          "maxclass": "newobj",
          "patching_rect": [
            20,
            390,
            110,
            22
          ],
          "numinlets": 1,
          "numoutlets": 3,
          "text": "live.thisdevice",
          "varname": "this-device"
        }
      },
      {
        "box": {
          "id": "ready-message",
          "maxclass": "message",
          "patching_rect": [
            20,
            420,
            50,
            22
          ],
          "numinlets": 1,
          "numoutlets": 1,
          "text": "ready"
        }
      },
      {
        "box": {
          "id": "active-message",
          "maxclass": "newobj",
          "patching_rect": [
            20,
            450,
            160,
            22
          ],
          "numinlets": 1,
          "numoutlets": 1,
          "text": "prepend deviceactive"
        }
      },
      {
        "box": {
          "id": "free",
          "maxclass": "newobj",
          "patching_rect": [
            20,
            480,
            65,
            22
          ],
          "numinlets": 1,
          "numoutlets": 1,
          "text": "freebang"
        }
      },
      {
        "box": {
          "id": "free-message",
          "maxclass": "message",
          "patching_rect": [
            20,
            510,
            65,
            22
          ],
          "numinlets": 1,
          "numoutlets": 1,
          "text": "freebang"
        }
      },
      {
        "box": {
          "id": "save-dialog",
          "maxclass": "newobj",
          "patching_rect": [
            930,
            260,
            100,
            22
          ],
          "numinlets": 1,
          "numoutlets": 3,
          "text": "savedialog",
          "varname": "save-dialog"
        }
      },
      {
        "box": {
          "id": "export-message",
          "maxclass": "newobj",
          "patching_rect": [
            930,
            290,
            150,
            22
          ],
          "numinlets": 1,
          "numoutlets": 1,
          "text": "prepend exportfile"
        }
      },
      {
        "box": {
          "id": "save-name",
          "maxclass": "newobj",
          "patching_rect": [
            930,
            230,
            190,
            22
          ],
          "numinlets": 1,
          "numoutlets": 1,
          "text": "loadmess name Submotif.mid"
        }
      },
      {
        "box": {
          "id": "state",
          "maxclass": "newobj",
          "patching_rect": [
            448,
            300,
            375,
            22
          ],
          "numinlets": 1,
          "numoutlets": 3,
          "text": "pattr patternState @bindto engine @parameter_enable 1",
          "varname": "patternState",
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Pattern State",
              "parameter_shortname": "Pattern State",
              "parameter_type": 3,
              "parameter_invisible": 1,
              "parameter_order": 100
            }
          },
          "saved_object_attributes": {
            "parameter_enable": 1
          }
        }
      }
    ],
    "lines": [
      {
        "patchline": {
          "source": [
            "root",
            0
          ],
          "destination": [
            "cfg-root",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "cfg-root",
            0
          ],
          "destination": [
            "engine",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "scale",
            0
          ],
          "destination": [
            "cfg-scale",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "cfg-scale",
            0
          ],
          "destination": [
            "engine",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "style",
            0
          ],
          "destination": [
            "cfg-style",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "cfg-style",
            0
          ],
          "destination": [
            "engine",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "bars",
            0
          ],
          "destination": [
            "cfg-bars",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "cfg-bars",
            0
          ],
          "destination": [
            "engine",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "octave",
            0
          ],
          "destination": [
            "cfg-octave",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "cfg-octave",
            0
          ],
          "destination": [
            "engine",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "density",
            0
          ],
          "destination": [
            "cfg-density",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "cfg-density",
            0
          ],
          "destination": [
            "engine",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "swing",
            0
          ],
          "destination": [
            "cfg-swing",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "cfg-swing",
            0
          ],
          "destination": [
            "engine",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "gate",
            0
          ],
          "destination": [
            "cfg-gate",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "cfg-gate",
            0
          ],
          "destination": [
            "engine",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "movement",
            0
          ],
          "destination": [
            "cfg-movement",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "cfg-movement",
            0
          ],
          "destination": [
            "engine",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "rootOnly",
            0
          ],
          "destination": [
            "cfg-rootOnly",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "cfg-rootOnly",
            0
          ],
          "destination": [
            "engine",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "anchorRoot",
            0
          ],
          "destination": [
            "cfg-anchorRoot",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "cfg-anchorRoot",
            0
          ],
          "destination": [
            "engine",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "octaveJumps",
            0
          ],
          "destination": [
            "cfg-octaveJumps",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "cfg-octaveJumps",
            0
          ],
          "destination": [
            "engine",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "generate",
            0
          ],
          "destination": [
            "action-generate",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "action-generate",
            0
          ],
          "destination": [
            "engine",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "vary",
            0
          ],
          "destination": [
            "action-vary",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "action-vary",
            0
          ],
          "destination": [
            "engine",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "undo",
            0
          ],
          "destination": [
            "action-undo",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "action-undo",
            0
          ],
          "destination": [
            "engine",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "redo",
            0
          ],
          "destination": [
            "action-redo",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "action-redo",
            0
          ],
          "destination": [
            "engine",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "clip",
            0
          ],
          "destination": [
            "action-clip",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "action-clip",
            0
          ],
          "destination": [
            "engine",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "export",
            0
          ],
          "destination": [
            "action-export",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "action-export",
            0
          ],
          "destination": [
            "engine",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "output",
            0
          ],
          "destination": [
            "cfg-output",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "cfg-output",
            0
          ],
          "destination": [
            "engine",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "panic",
            0
          ],
          "destination": [
            "action-panic",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "action-panic",
            0
          ],
          "destination": [
            "engine",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "unlock",
            0
          ],
          "destination": [
            "action-unlock",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "action-unlock",
            0
          ],
          "destination": [
            "engine",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "more",
            0
          ],
          "destination": [
            "action-more",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "action-more",
            0
          ],
          "destination": [
            "engine",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "midi-in",
            0
          ],
          "destination": [
            "thru",
            1
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "engine",
            2
          ],
          "destination": [
            "thru",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "thru",
            0
          ],
          "destination": [
            "midi-out",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "engine",
            0
          ],
          "destination": [
            "midi-out",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "engine",
            1
          ],
          "destination": [
            "roll",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "roll",
            0
          ],
          "destination": [
            "engine",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "clock",
            0
          ],
          "destination": [
            "transport-reader",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "transport-reader",
            7
          ],
          "destination": [
            "position-message",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "position-message",
            0
          ],
          "destination": [
            "engine",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "transport-reader",
            6
          ],
          "destination": [
            "playing-message",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "playing-message",
            0
          ],
          "destination": [
            "engine",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "transport-reader",
            0
          ],
          "destination": [
            "pulse-message",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "pulse-message",
            0
          ],
          "destination": [
            "engine",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "this-device",
            0
          ],
          "destination": [
            "ready-message",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "ready-message",
            0
          ],
          "destination": [
            "engine",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "this-device",
            1
          ],
          "destination": [
            "active-message",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "active-message",
            0
          ],
          "destination": [
            "engine",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "free",
            0
          ],
          "destination": [
            "free-message",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "free-message",
            0
          ],
          "destination": [
            "engine",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "engine",
            3
          ],
          "destination": [
            "save-dialog",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "save-dialog",
            0
          ],
          "destination": [
            "export-message",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "export-message",
            0
          ],
          "destination": [
            "engine",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "save-name",
            0
          ],
          "destination": [
            "save-dialog",
            0
          ]
        }
      }
    ],
    "parameters": {
      "root": [
        "Key",
        "Key",
        0
      ],
      "scale": [
        "Scale",
        "Scale",
        0
      ],
      "style": [
        "Style",
        "Style",
        0
      ],
      "bars": [
        "Bars",
        "Bars",
        0
      ],
      "octave": [
        "Octave",
        "Octave",
        0
      ],
      "density": [
        "Density",
        "Density",
        0
      ],
      "swing": [
        "Swing",
        "Swing",
        0
      ],
      "gate": [
        "Gate",
        "Gate",
        0
      ],
      "movement": [
        "Movement",
        "Movement",
        0
      ],
      "rootOnly": [
        "ROOT ONLY",
        "ROOT ONLY",
        0
      ],
      "anchorRoot": [
        "ROOT ANCHOR",
        "ROOT ANCHOR",
        0
      ],
      "octaveJumps": [
        "OCTAVE JUMPS",
        "OCTAVE JUMPS",
        0
      ],
      "generate": [
        "GENERATE",
        "GENERATE",
        0
      ],
      "vary": [
        "Variation",
        "Variation",
        0
      ],
      "undo": [
        "Undo",
        "Undo",
        0
      ],
      "redo": [
        "Redo",
        "Redo",
        0
      ],
      "clip": [
        "CREATE CLIP",
        "CREATE CLIP",
        0
      ],
      "export": [
        "SAVE MIDI",
        "SAVE MIDI",
        0
      ],
      "output": [
        "MIDI ON",
        "MIDI ON",
        0
      ],
      "panic": [
        "Panic",
        "Panic",
        0
      ],
      "unlock": [
        "Unlock notes",
        "Unlock notes",
        0
      ],
      "more": [
        "More controls",
        "More controls",
        0
      ],
      "state": [
        "Pattern State",
        "Pattern State",
        0
      ],
      "parameterbanks": {
        "0": {
          "index": 0,
          "name": "Groove",
          "parameters": [
            "density",
            "swing",
            "gate",
            "movement",
            "root",
            "scale",
            "style",
            "bars"
          ]
        }
      },
      "inherited_shortname": 1
    },
    "latency": 0,
    "dependency_cache": [
      {
        "name": "undertow.js",
        "bootpath": ".",
        "patcherrelativepath": ".",
        "type": "TEXT",
        "implicit": 1
      },
      {
        "name": "undertow-core.js",
        "bootpath": ".",
        "patcherrelativepath": ".",
        "type": "TEXT",
        "implicit": 1
      },
      {
        "name": "undertow-roll.js",
        "bootpath": ".",
        "patcherrelativepath": ".",
        "type": "TEXT",
        "implicit": 1
      }
    ],
    "project": {
      "version": 1,
      "autoorganize": 0,
      "hideprojectwindow": 1,
      "showdependencies": 1,
      "autolocalize": 0,
      "contents": {
        "patchers": {},
        "code": {
          "undertow.js": {
            "kind": "javascript",
            "local": 1
          },
          "undertow-core.js": {
            "kind": "javascript",
            "local": 1
          },
          "undertow-roll.js": {
            "kind": "javascript",
            "local": 1
          }
        }
      },
      "searchpath": {},
      "amxdtype": 1835887981,
      "readonly": 0,
      "devpathtype": 0,
      "devpath": "."
    },
    "autosave": 0
  }
}
