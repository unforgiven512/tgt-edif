library_cell xilinx_spartan_library[]={
	/* auxiliary cells */
	{ "XILINX_LUT1", "LUT1", "netlist",
		"\t\t(cell LUT1 (celltype GENERIC) (view netlist (viewtype NETLIST)"
		" (interface (port I0 (direction INPUT))"
		" (port O (direction OUTPUT))"
		" )))\n",
		2,
		(char *[]){"O", "I0"} } ,
	{ "XILINX_LUT2", "LUT2", "netlist",
		"\t\t(cell LUT2 (celltype GENERIC) (view netlist (viewtype NETLIST)"
		" (interface (port I0 (direction INPUT)) (port I1 (direction INPUT))"
		" (port O (direction OUTPUT))"
		" )))\n",
		3,
		(char *[]){"O", "I0", "I1"} } ,
	{ "XILINX_LUT3", "LUT3", "netlist",
		"\t\t(cell LUT3 (celltype GENERIC) (view netlist (viewtype NETLIST)"
		" (interface (port I0 (direction INPUT)) (port I1 (direction INPUT)) (port I2 (direction INPUT))"
		" (port O (direction OUTPUT))"
		" )))\n",
		4,
		(char *[]){"O", "I0", "I1", "I2"} } ,
	{ "ZERO" , "GND", "netlist",
		"\t\t(cell GND (cellType GENERIC) (view netlist (viewType NETLIST)"
            	" (interface (port G (direction OUTPUT))"
            	" )))\n",
		1,
		(char *[]){ "G" } } ,
	{ "ONE" , "VCC", "netlist",
		"\t\t(cell VCC (cellType GENERIC) (view netlist (viewType NETLIST)"
            	" (interface (port P (direction OUTPUT))"
            	" )))\n",
		1,
		(char *[]){ "P" } } ,
	{ "NOT" , "INV", "INV",
		"\t\t(cell INV (cellType GENERIC) (view INV (viewType NETLIST)"
            	" (interface (port A (direction INPUT)) (port QN (direction OUTPUT))"
            	" )\n"
		"\t\t\t(contents\n"
		"\t\t\t\t(instance inv (viewref netlist (cellref LUT1)) (property INIT (string \"1\")))\n"
		"\t\t\t\t(net (joined (portRef A) (portRef I0 (instanceRef inv))))\n"
		"\t\t\t\t(net (joined (portRef QN) (portRef O (instanceRef inv))))\n"
		"\t\t\t\t)))\n",
		2,
		(char *[]){ "QN", "A"} } ,
	{ "AND" , "AN2", "AN2",
		"\t\t(cell AN2 (cellType GENERIC) (view AN2 (viewType NETLIST)"
            	" (interface (port A (direction INPUT)) (port B (direction INPUT)) (port Q (direction OUTPUT)))"
		"\t\t\t(contents\n"
		"\t\t\t\t(instance lut2 (viewref netlist (cellref LUT2)) (property INIT (string \"8\")))\n"
		"\t\t\t\t(net (joined (portRef A) (portRef I0 (instanceRef lut2))))\n"
		"\t\t\t\t(net (joined (portRef B) (portRef I1 (instanceRef lut2))))\n"
		"\t\t\t\t(net (joined (portRef Q) (portRef O (instanceRef lut2))))\n"
		"\t\t\t\t)))\n",
		3,
		(char *[]){ "Q", "A", "B" } } ,
	{ "OR" , "OR2", "OR2",
		"\t\t(cell OR2 (cellType GENERIC) (view OR2 (viewType NETLIST)"
            	" (interface (port A (direction INPUT)) (port B (direction INPUT)) (port Q (direction OUTPUT)))"
		"\t\t\t(contents\n"
		"\t\t\t\t(instance lut2 (viewref netlist (cellref LUT2)) (property INIT (string \"E\")))\n"
		"\t\t\t\t(net (joined (portRef A) (portRef I0 (instanceRef lut2))))\n"
		"\t\t\t\t(net (joined (portRef B) (portRef I1 (instanceRef lut2))))\n"
		"\t\t\t\t(net (joined (portRef Q) (portRef O (instanceRef lut2))))\n"
		"\t\t\t\t)))\n",
		3,
		(char *[]){ "Q", "A", "B" } } ,
	{ "XOR" , "XO2", "XO2",
		"\t\t(cell XO2 (cellType GENERIC) (view XO2 (viewType NETLIST)"
            	" (interface (port A (direction INPUT)) (port B (direction INPUT)) (port Q (direction OUTPUT)))"
		"\t\t\t(contents\n"
		"\t\t\t\t(instance lut2 (viewref netlist (cellref LUT2)) (property INIT (string \"6\")))\n"
		"\t\t\t\t(net (joined (portRef A) (portRef I0 (instanceRef lut2))))\n"
		"\t\t\t\t(net (joined (portRef B) (portRef I1 (instanceRef lut2))))\n"
		"\t\t\t\t(net (joined (portRef Q) (portRef O (instanceRef lut2))))\n"
		"\t\t\t\t)))\n",
		3,
		(char *[]){ "Q", "A", "B" } } ,
	{ "XNOR" , "XN2", "XN2",
		"\t\t(cell XN2 (cellType GENERIC) (view XN2 (viewType NETLIST)"
            	" (interface (port A (direction INPUT)) (port B (direction INPUT)) (port Q (direction OUTPUT)))"
		"\t\t\t(contents\n"
		"\t\t\t\t(instance lut2 (viewref netlist (cellref LUT2)) (property INIT (string \"9\")))\n"
		"\t\t\t\t(net (joined (portRef A) (portRef I0 (instanceRef lut2))))\n"
		"\t\t\t\t(net (joined (portRef B) (portRef I1 (instanceRef lut2))))\n"
		"\t\t\t\t(net (joined (portRef Q) (portRef O (instanceRef lut2))))\n"
		"\t\t\t\t)))\n",
		3,
		(char *[]){ "Q", "A", "B" } } ,
	{ "NOR" , "NO2", "NO2",
		"\t\t(cell NO2 (cellType GENERIC) (view NO2 (viewType NETLIST)"
            	" (interface (port A (direction INPUT)) (port B (direction INPUT)) (port Q (direction OUTPUT)))"
		"\t\t\t(contents\n"
		"\t\t\t\t(instance lut2 (viewref netlist (cellref LUT2)) (property INIT (string \"1\")))\n"
		"\t\t\t\t(net (joined (portRef A) (portRef I0 (instanceRef lut2))))\n"
		"\t\t\t\t(net (joined (portRef B) (portRef I1 (instanceRef lut2))))\n"
		"\t\t\t\t(net (joined (portRef Q) (portRef O (instanceRef lut2))))\n"
		"\t\t\t\t)))\n",
		3,
		(char *[]){ "Q", "A", "B" } } ,
	{ "NAND" , "NA2", "NA2",
		"\t\t(cell NA2 (cellType GENERIC) (view NA2 (viewType NETLIST)"
            	" (interface (port A (direction INPUT)) (port B (direction INPUT)) (port Q (direction OUTPUT)))"
		"\t\t\t(contents\n"
		"\t\t\t\t(instance lut2 (viewref netlist (cellref LUT2)) (property INIT (string \"7\")))\n"
		"\t\t\t\t(net (joined (portRef A) (portRef I0 (instanceRef lut2))))\n"
		"\t\t\t\t(net (joined (portRef B) (portRef I1 (instanceRef lut2))))\n"
		"\t\t\t\t(net (joined (portRef Q) (portRef O (instanceRef lut2))))\n"
		"\t\t\t\t)))\n",
		3,
		(char *[]){ "Q", "A", "B" } } ,
	{ "IBUF", "IBUF", "netlist",
		"\t\t(cell IBUF (cellType GENERIC) (view netlist (viewType NETLIST)"
		" (interface (port O (direction OUTPUT)) (port I (direction INPUT))"
		" (property MODEL (string \"IBUF\")))))\n",
		2,
		(char *[]){"I", "O"} },
	{ "GCLKBUF", "BUFGP", "netlist",
		"\t\t(cell BUFGP (cellType GENERIC) (view netlist (viewType NETLIST)"
		" (interface (port Q (direction OUTPUT)) (port PAD (direction INPUT))"
		" (property MODEL (string \"GCLKBUF\")))))\n",
		2,
		(char *[]){"PAD", "Q"} },
/*	{ "RSBUF", "RSBUF", "RSBUF",
		"\t\t(cell RSBUF (cellType GENERIC) (view RSBUF (viewType NETLIST)"
		" (interface (port Q (direction OUTPUT)) (port PAD (direction INPUT))"
		" (property MODEL (string \"RSBUF\")))))\n",
		2,
		(char *[]){"PAD", "Q"} }, */
        { "OBUF", "OBUF", "netlist",
                "\t\t(cell OBUF (cellType GENERIC) (view netlist (viewType NETLIST)"
                " (interface (port I (direction INPUT)) (port O (direction OUTPUT))"
                " (property MODEL (string \"OBUF\")))))\n",
                2,
                (char *[]){"O", "I"} },
		/* Xilinx's BUFZ is nothing else but bufif */
	{ "BUFIF" , "BUFT", "netlist",
		"\t\t(cell BUFZ (cellType GENERIC) (view netlist (viewType NETLIST)"
            	" (interface (port I (direction INPUT)) (port T (direction INPUT)) (port O (direction OUTPUT))"
            	" )))\n",
		3,
		(char *[]){ "O", "I", "O" } } , 
	/* instead let us model it as bufif with OE being constant 1 */
        { "BUFZ", "IVERILOG_MY_BUFZ", "IVERILOG_MY_BUFZ",
                "\t\t(cell IVERILOG_MY_BUFZ (cellType GENERIC) (view IVERILOG_MY_BUFZ (viewType NETLIST)"
                "\t\t\t(interface (port A (direction INPUT)) (port Q (direction OUTPUT)))\n"
		"\t\t\t(contents\n"
		"\t\t\t\t(instance B (viewRef netlist (cellRef BUFT)))\n"
		"\t\t\t\t(instance V (viewRef netlist (cellRef GND)))\n"
		"\t\t\t\t(net A (joined (portRef A) (portRef I (instanceRef B))))\n"
		"\t\t\t\t(net Q (joined (portRef Q) (portRef O (instanceRef B))))\n"
		"\t\t\t\t(net V (joined (portRef O (instanceRef V)) (portRef T (instanceRef B))))\n"
                "\t\t\t\t)))\n",
                2,
                (char *[]){"Q", "A"} },
        { "BIBUF", "OBUFT", "netlist",
                "\t\t(cell OBUFT (cellType GENERIC) (view netlist (viewType NETLIST)"
                " (interface (port I (direction INPUT)) (port O (direction OUTPUT)) (port T (direction INPUT))"
                " (property MODEL (string \"OBUF\")))))\n",
                3,
                (char *[]){"O", "I", "T"} },
	{ "MUX" , "MUX2", "MUX2",
		"\t\t(cell MUX2 (cellType GENERIC) (view MUX2 (viewType NETLIST)"
            	" (interface (port D0 (direction INPUT)) (port D1 (direction INPUT)) (port S (direction INPUT)) (port Q (direction OUTPUT)))\n"
 		"\t\t\t(contents\n"
		"\t\t\t\t(instance lut3 (viewRef netlist (cellRef LUT3)) (property INIT (string \"D8\")))"
		"\t\t\t\t(net d0 (portRef D0) (portRef I1 (instanceRef lut3)))\n"
		"\t\t\t\t(net d1 (portRef D1) (portRef I2 (instanceRef lut3)))\n"
		"\t\t\t\t(net s (portRef S) (portRef I0 (instanceRef lut3)))\n"
		"\t\t\t\t(net q (portRef Q) (portRef O (instanceRef lut3)))\n"
		"\t\t\t))\n",
		4,
		(char *[]){ "Q", "D0", "D1", "S" } } ,
	{ "DFF" , "FD", "netlist",
		"\t\t(cell FD (cellType GENERIC) (view netlist (viewType NETLIST)"
            	" (interface\n"
		"\t\t\t(port D (direction INPUT))\n"
		"\t\t\t(port C (direction INPUT))\n"
		"\t\t\t(port Q (direction OUTPUT)))\n"
		"\t\t\t))\n",
		3,
		(char *[]){ "Q", "D", "C"} } ,
	/* edited up to here */
	/* I don't seem to find an ADDC cell.. make one up */
	{ "ADDC", "IVERILOG_ADDC", "netlist",
		"\t\t(cell IVERILOG_ADDC (cellType GENERIC) (view netlist (viewType NETLIST)"
            	" (interface\n"
		"\t\t\t(port A (direction INPUT))\n"
		"\t\t\t(port B (direction INPUT))\n"
		"\t\t\t(port CI (direction INPUT))\n"
		"\t\t\t(port SUM (direction OUTPUT))\n"
		"\t\t\t(port COUT (direction OUTPUT))\n"
            	"\t\t\t(property MODEL (string \"ADDC\")))\n"
		"\t\t\t(contents\n"
		"\t\t\t\t(instance sum_cell (viewRef netlist (cellRef LUT3)) (property INIT (string \"96\")))\n"
		"\t\t\t\t(instance cout_cell (viewRef netlist (cellRef LUT3)) (property INIT (string \"E8\")))\n"
		"\t\t\t\t(net a (joined (portRef A) (portRef I0 (instanceRef sum_cell)) (portRef I0 (instanceRef cout_cell))))\n"
		"\t\t\t\t(net b (joined (portRef B) (portRef I1 (instanceRef sum_cell)) (portRef I1 (instanceRef cout_cell))))\n"
		"\t\t\t\t(net cin (joined (portRef CIN) (portRef I2 (instanceRef sum_cell)) (portRef I2 (instanceRef cout_cell))))\n"
		"\t\t\t\t(net cout (joined (portRef COUT) (portRef O (instanceRef cout_cell))))\n"
		"\t\t\t\t(net sum (joined (portRef SUM) (portRef O (instanceRef sum_cell))))\n"
		"\t\t\t\t)))\n",
		5,
		(char *[]){ "SUM", "COUT", "A", "B", "CI"} } , 
	

	{ NULL, NULL, NULL, NULL, 0, NULL } };
 
