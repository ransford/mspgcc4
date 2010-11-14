# This text should be extracted from msp430.c
text = '''
	/* F1xx family */
	{"msp430x110",   MSP430_ISA_11, 0},
	{"msp430x112",   MSP430_ISA_11, 0},

	{"msp430x1101",  MSP430_ISA_110, 0},
	{"msp430x1111",  MSP430_ISA_110, 0},
	{"msp430x1121",  MSP430_ISA_110, 0},
	{"msp430x1122",  MSP430_ISA_110, 0},
	{"msp430x1132",  MSP430_ISA_110, 0},

	{"msp430x122",   MSP430_ISA_12, 0},
	{"msp430x123",   MSP430_ISA_12, 0},
	{"msp430x1222",  MSP430_ISA_12, 0},
	{"msp430x1232",  MSP430_ISA_12, 0},

	{"msp430x133",   MSP430_ISA_13, 0},
	{"msp430x135",   MSP430_ISA_13, 0},
	{"msp430x1331",  MSP430_ISA_13, 0},
	{"msp430x1351",  MSP430_ISA_13, 0},

	{"msp430x147",   MSP430_ISA_14, 1},
	{"msp430x148",   MSP430_ISA_14, 1},
	{"msp430x149",   MSP430_ISA_14, 1},
	{"msp430x1471",  MSP430_ISA_14, 1},
	{"msp430x1481",  MSP430_ISA_14, 1},
	{"msp430x1491",  MSP430_ISA_14, 1},

	{"msp430x155",   MSP430_ISA_15, 0},
	{"msp430x156",   MSP430_ISA_15, 0},
	{"msp430x157",   MSP430_ISA_15, 0},

	{"msp430x167",   MSP430_ISA_16, 1},
	{"msp430x168",   MSP430_ISA_16, 1},
	{"msp430x169",   MSP430_ISA_16, 1},
	{"msp430x1610",  MSP430_ISA_16, 1},
	{"msp430x1611",  MSP430_ISA_16, 1},
	{"msp430x1612",  MSP430_ISA_16, 1},

	/* F2xx family */
	{"msp430x2001",  MSP430_ISA_20, 0},
	{"msp430x2011",  MSP430_ISA_20, 0},

	{"msp430x2002",  MSP430_ISA_20, 0},
	{"msp430x2012",  MSP430_ISA_20, 0},

	{"msp430x2003",  MSP430_ISA_20, 0},
	{"msp430x2013",  MSP430_ISA_20, 0},

	{"msp430x2101",  MSP430_ISA_21, 0},
	{"msp430x2111",  MSP430_ISA_21, 0},
	{"msp430x2121",  MSP430_ISA_21, 0},
	{"msp430x2131",  MSP430_ISA_21, 0},

	{"msp430x2112",  MSP430_ISA_22, 0},
	{"msp430x2122",  MSP430_ISA_22, 0},
	{"msp430x2132",  MSP430_ISA_22, 0},

	{"msp430x2201",  MSP430_ISA_22, 0}, /* Value-Line */
	{"msp430x2211",  MSP430_ISA_22, 0}, /* Value-Line */
	{"msp430x2221",  MSP430_ISA_22, 0}, /* Value-Line */
	{"msp430x2231",  MSP430_ISA_22, 0}, /* Value-Line */

	{"msp430x2232",  MSP430_ISA_22, 0},
	{"msp430x2252",  MSP430_ISA_22, 0},
	{"msp430x2272",  MSP430_ISA_22, 0},

	{"msp430x2234",  MSP430_ISA_22, 0},
	{"msp430x2254",  MSP430_ISA_22, 0},
	{"msp430x2274",  MSP430_ISA_22, 0},

	{"msp430x233",   MSP430_ISA_23, 1},
	{"msp430x235",   MSP430_ISA_23, 1},

	{"msp430x2330",  MSP430_ISA_23, 1},
	{"msp430x2350",  MSP430_ISA_23, 1},
	{"msp430x2370",  MSP430_ISA_23, 1},

	{"msp430x247",   MSP430_ISA_24, 1},
	{"msp430x248",   MSP430_ISA_24, 1},
	{"msp430x249",   MSP430_ISA_24, 1},
	{"msp430x2410",  MSP430_ISA_24, 1},
	{"msp430x2471",  MSP430_ISA_24, 1},
	{"msp430x2481",  MSP430_ISA_24, 1},
	{"msp430x2491",  MSP430_ISA_24, 1},

	{"msp430x2416",  MSP430_ISA_241, 1},
	{"msp430x2417",  MSP430_ISA_241, 1},
	{"msp430x2418",  MSP430_ISA_241, 1},
	{"msp430x2419",  MSP430_ISA_241, 1},

	{"msp430x2616",  MSP430_ISA_26, 1},
	{"msp430x2617",  MSP430_ISA_26, 1},
	{"msp430x2618",  MSP430_ISA_26, 1},
	{"msp430x2619",  MSP430_ISA_26, 1},

	/* 3xx family (ROM) */
	{"msp430x311",   MSP430_ISA_31, 0},
	{"msp430x312",   MSP430_ISA_31, 0},
	{"msp430x313",   MSP430_ISA_31, 0},
	{"msp430x314",   MSP430_ISA_31, 0},
	{"msp430x315",   MSP430_ISA_31, 0},

	{"msp430x323",   MSP430_ISA_32, 0},
	{"msp430x325",   MSP430_ISA_32, 0},

	{"msp430x336",   MSP430_ISA_33, 1},
	{"msp430x337",   MSP430_ISA_33, 1},

	/* F4xx family */
	{"msp430x412",   MSP430_ISA_41, 0},
	{"msp430x413",   MSP430_ISA_41, 0},
	{"msp430x415",   MSP430_ISA_41, 0},
	{"msp430x417",   MSP430_ISA_41, 0},

	{"msp430x423",   MSP430_ISA_42, 1},
	{"msp430x425",   MSP430_ISA_42, 1},
	{"msp430x427",   MSP430_ISA_42, 1},

	{"msp430x4250",  MSP430_ISA_42, 0},
	{"msp430x4260",  MSP430_ISA_42, 0},
	{"msp430x4270",  MSP430_ISA_42, 0},

	{"msp430xG4250", MSP430_ISA_42, 0},
	{"msp430xG4260", MSP430_ISA_42, 0},
	{"msp430xG4270", MSP430_ISA_42, 0},

	{"msp430xE423",  MSP430_ISA_42, 1},
	{"msp430xE425",  MSP430_ISA_42, 1},
	{"msp430xE427",  MSP430_ISA_42, 1},

	{"msp430xE4232", MSP430_ISA_42, 1},
	{"msp430xE4242", MSP430_ISA_42, 1},
	{"msp430xE4252", MSP430_ISA_42, 1},
	{"msp430xE4272", MSP430_ISA_42, 1},

	{"msp430xW423",  MSP430_ISA_42, 0},
	{"msp430xW425",  MSP430_ISA_42, 0},
	{"msp430xW427",  MSP430_ISA_42, 0},

	{"msp430xG437",  MSP430_ISA_43, 0},
	{"msp430xG438",  MSP430_ISA_43, 0},
	{"msp430xG439",  MSP430_ISA_43, 0},

	{"msp430x435",   MSP430_ISA_43, 0},
	{"msp430x436",   MSP430_ISA_43, 0},
	{"msp430x437",   MSP430_ISA_43, 0},

	{"msp430x4351",  MSP430_ISA_43, 0},
	{"msp430x4361",  MSP430_ISA_43, 0},
	{"msp430x4371",  MSP430_ISA_43, 0},

	{"msp430x447",   MSP430_ISA_44, 1},
	{"msp430x448",   MSP430_ISA_44, 1},
	{"msp430x449",   MSP430_ISA_44, 1},

	{"msp430xG4616", MSP430_ISA_46, 1},
	{"msp430xG4617", MSP430_ISA_46, 1},
	{"msp430xG4618", MSP430_ISA_46, 1},
	{"msp430xG4619", MSP430_ISA_46, 1},

	{"msp430x4783",  MSP430_ISA_47, 1},
	{"msp430x4784",  MSP430_ISA_47, 1},
	{"msp430x4793",  MSP430_ISA_47, 1},
	{"msp430x4794",  MSP430_ISA_47, 1},
        
	{"msp430x47166", MSP430_ISA_471, 1},
	{"msp430x47176", MSP430_ISA_471, 1},
	{"msp430x47186", MSP430_ISA_471, 1},
	{"msp430x47196", MSP430_ISA_471, 1},

	{"msp430x47167", MSP430_ISA_471, 1},
	{"msp430x47177", MSP430_ISA_471, 1},
	{"msp430x47187", MSP430_ISA_471, 1},
	{"msp430x47197", MSP430_ISA_471, 1},

	/* F5xxx family */
	{"msp430x5418",  MSP430_ISA_54, 1},
	{"msp430x5419",  MSP430_ISA_54, 1},
	{"msp430x5435",  MSP430_ISA_54, 1},
	{"msp430x5436",  MSP430_ISA_54, 1},
	{"msp430x5437",  MSP430_ISA_54, 1},
	{"msp430x5438",  MSP430_ISA_54, 1},

        {"msp430x5510",  MSP430_ISA_54, 1},
        {"msp430x5513",  MSP430_ISA_54, 1},
        {"msp430x5514",  MSP430_ISA_54, 1},
        {"msp430x5515",  MSP430_ISA_54, 1},
        {"msp430x5517",  MSP430_ISA_54, 1},
        {"msp430x5519",  MSP430_ISA_54, 1},
        {"msp430x5521",  MSP430_ISA_54, 1},
        {"msp430x5522",  MSP430_ISA_54, 1},
        {"msp430x5524",  MSP430_ISA_54, 1},
        {"msp430x5525",  MSP430_ISA_54, 1},
        {"msp430x5526",  MSP430_ISA_54, 1},
        {"msp430x5527",  MSP430_ISA_54, 1},
        {"msp430x5528",  MSP430_ISA_54, 1},
        {"msp430x5529",  MSP430_ISA_54, 1},
        {"msp430x6638",  MSP430_ISA_54, 1},

	/* CC430 family */
	{"cc430x5133",   MSP430_ISA_54, 1},
	{"cc430x5125",   MSP430_ISA_54, 1},
	{"cc430x6125",   MSP430_ISA_54, 1},
	{"cc430x6135",   MSP430_ISA_54, 1},
	{"cc430x6126",   MSP430_ISA_54, 1},
	{"cc430x5137",   MSP430_ISA_54, 1},
	{"cc430x6127",   MSP430_ISA_54, 1},
	{"cc430x6137",   MSP430_ISA_54, 1},
'''

import re

pattern_re = re.compile('.*"(.*)".*MSP430_ISA_(\d*),\s*([01])')

exemplary_chip = { 'msp1' : 'msp430x110',
                   'msp2' : 'msp430x336',
                   'msp3' : 'msp430xG4616',
                   'msp4' : 'msp430x4783',
                   'msp5' : 'msp430x47166',
                   'msp6' : 'msp430x5418' }

generated =  { }
seen_chip = { }

for line in text.splitlines():
    mo = pattern_re.match(line)
    if mo is None:
        continue
    (chip, ctype, hwm) = mo.groups()
    ctype = int(ctype)
    hwm = int(hwm)
    '''
    if hwm:
        print 'mmcu?msp2=mmcu?%s' % (chip,)
    else:
        print 'mmcu?msp1=mmcu?%s' % (chip,)
    '''
    mlib = None
    if not hwm:
        mlib = 'msp1'
    else:
        if ctype in (26, 241, 46,):
            mlib = 'msp3'
        elif ctype in (47,):
            mlib = 'msp4'
        elif ctype in (471,):
            mlib = 'msp5'
        elif ctype in (54,):
            mlib = 'msp6'
        else:
            mlib = 'msp2'
    aux_cbs = ''
    if (exemplary_chip[mlib] == chip):
        aux_cbs = '|mmcu=%s' % (mlib,)
    generated.setdefault('MULTILIB_MATCHES', []).append("\tmmcu?%s=mmcu?%s" % (mlib, chip))
    generated.setdefault('CPP_SPEC', []).append('%%{mmcu=%s:%%(cpp_%s) -D__%s__}' % (chip, mlib, chip.replace('x', '_').upper()))
    generated.setdefault('LINK_SPEC', []).append('%%{mmcu=%s:-m %s }' % (chip, chip))
    generated.setdefault('CRT_BINUTILS_SPECS', []).append('%%{mmcu=%s%s:crt%s.o%%s}' % (chip, aux_cbs, chip.replace('msp', '')))
    
for (k, v) in generated.items():
    file(k, 'w').write(" \\\n".join(v))
