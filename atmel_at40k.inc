library_cell atmel_at40k_library[] = {
	{
		"ZERO", "ZERO", "ZERO",
		"\t\t(cell ZERO (cellType GENERIC) (view ZERO (viewType NETLIST)"
				" (interface (port Q (direction OUTPUT))"
				" (property MODEL (string \"ZERO\")))))\n",
		1,
		(char *[]){ "Q" }
	}, {
		"ONE", "ONE", "ONE",
		"\t\t(cell ONE (cellType GENERIC) (view ONE (viewType NETLIST)"
				" (interface (port Q (direction OUTPUT))"
				" (property MODEL (string \"ONE\")))))\n",
		1,
		(char *[]){ "Q" }
	}, {
		"NOT", "INV", "INV",
		"\t\t(cell INV (cellType GENERIC) (view INV (viewType NETLIST)"
				" (interface (port A (direction INPUT)) (port QN (direction OUTPUT))"
				" (property MODEL (string \"INV\")))))\n",
		2,
		(char *[]){ "QN", "A" }
	}, {
		"AND", "AN2", "AN2",
		"\t\t(cell AN2 (cellType GENERIC) (view AN2 (viewType NETLIST)"
				" (interface (port A (direction INPUT)) (port B (direction INPUT)) (port Q (direction OUTPUT))"
				" (property MODEL (string \"AND\")))))\n",
		3,
		(char *[]){ "Q", "A", "B" }
	}, {
		"OR", "OR2", "OR2",
		"\t\t(cell OR2 (cellType GENERIC) (view OR2 (viewType NETLIST)"
				" (interface (port A (direction INPUT)) (port B (direction INPUT)) (port Q (direction OUTPUT))"
				" (property MODEL (string \"OR\")))))\n",
		3,
		(char *[]){ "Q", "A", "B" }
	}, {
		"XOR", "XO2", "XO2",
		"\t\t(cell XO2 (cellType GENERIC) (view XO2 (viewType NETLIST)"
				" (interface (port A (direction INPUT)) (port B (direction INPUT)) (port Q (direction OUTPUT))"
				" (property MODEL (string \"XOR\")))))\n",
		3,
		(char *[]){ "Q", "A", "B" }
	}, {
		"XNOR", "XN2", "XN2",
		"\t\t(cell XN2 (cellType GENERIC) (view XN2 (viewType NETLIST)"
				" (interface (port A (direction INPUT)) (port B (direction INPUT)) (port Q (direction OUTPUT))"
				" (property MODEL (string \"XNOR\")))))\n",
		3,
		(char *[]){ "Q", "A", "B" }
	}, {
		"NOR", "NO2", "NO2",
		"\t\t(cell NO2 (cellType GENERIC) (view NO2 (viewType NETLIST)"
				" (interface (port A (direction INPUT)) (port B (direction INPUT)) (port Q (direction OUTPUT))"
				" (property MODEL (string \"NOR\")))))\n",
		3,
		(char *[]){ "Q", "A", "B" }
	}, {
		"NAND", "NA2", "NA2",
		"\t\t(cell NA2 (cellType GENERIC) (view NA2 (viewType NETLIST)"
				" (interface (port A (direction INPUT)) (port B (direction INPUT)) (port Q (direction OUTPUT))"
				" (property MODEL (string \"NAND\")))))\n",
		3,
		(char *[]){ "Q", "A", "B" }
	}, {
		"IBUF", "IBUF", "IBUF",
		"\t\t(cell IBUF (cellType GENERIC) (view IBUF (viewType NETLIST)"
				" (interface (port Q (direction OUTPUT)) (port PAD (direction INPUT))"
				" (property MODEL (string \"IBUF\")))))\n",
		2,
		(char *[]){ "PAD", "Q" }
	}, {
		"GCLKBUF", "GCLKBUF", "GCLKBUF",
		"\t\t(cell GCLKBUF (cellType GENERIC) (view GCLKBUF (viewType NETLIST)"
				" (interface (port Q (direction OUTPUT)) (port PAD (direction INPUT))"
				" (property MODEL (string \"GCLKBUF\")))))\n",
		2,
		(char *[]){ "PAD", "Q" }
	}, {
		"RSBUF", "RSBUF", "RSBUF",
		"\t\t(cell RSBUF (cellType GENERIC) (view RSBUF (viewType NETLIST)"
				" (interface (port Q (direction OUTPUT)) (port PAD (direction INPUT))"
				" (property MODEL (string \"RSBUF\")))))\n",
		2,
		(char *[]){ "PAD", "Q" }
	}, {
		"OBUF", "OBUF", "OBUF",
		"\t\t(cell OBUF (cellType GENERIC) (view OBUF (viewType NETLIST)"
				" (interface (port A (direction INPUT)) (port PAD (direction OUTPUT))"
				" (property MODEL (string \"OBUF\")))))\n",
		2,
		(char *[]){ "PAD", "A" }
	}, {
		/* Atmel's BUFZ is nothing else but bufif */
		"BUFIF", "BUFZ", "BUFZ",
		"\t\t(cell BUFZ (cellType GENERIC) (view BUFZ (viewType NETLIST)"
				" (interface (port A (direction INPUT)) (port OE (direction INPUT)) (port Q (direction OUTPUT))"
				" (property MODEL (string \"BUFIF\")))))\n",
		3,
		(char *[]){ "Q", "A", "OE" }
	}, {
	/* I have big doubts that BUFZ is described as OBUF */
/* ----- *
		"BUFZ", "BUFZ", "BUFZ",
		"\t\t(cell OBUF (cellType GENERIC) (view OBUF (viewType NETLIST)"
				" (interface (port A (direction INPUT)) (port PAD (direction OUTPUT))"
				" (property MODEL (string \"OBUF\")))))\n",
		2,
		(char *[]){ "PAD", "A" }
	}, {
 * ----- */
		/* instead let us model it as bufif with OE being constant 1 */
		"BUFZ", "IVERILOG_MY_BUFZ", "IVERILOG_MY_BUFZ",
		"\t\t(cell IVERILOG_MY_BUFZ (cellType GENERIC) (view IVERILOG_MY_BUFZ (viewType NETLIST)"
				"\t\t\t(interface (port A (direction INPUT)) (port Q (direction OUTPUT)))\n"
				"\t\t\t(contents\n"
				"\t\t\t\t(instance B (viewRef BUFZ (cellRef BUFZ)))\n"
				"\t\t\t\t(instance V (viewRef ONE (cellRef ONE)))\n"
				"\t\t\t\t(net A (joined (portRef A) (portRef A (instanceRef B))))\n"
				"\t\t\t\t(net Q (joined (portRef Q) (portRef Q (instanceRef B))))\n"
				"\t\t\t\t(net V (joined (portRef Q (instanceRef V)) (portRef OE (instanceRef B))))\n"
				"\t\t\t\t)))\n",
		2,
		(char *[]){ "Q", "A" }
	}, {
		"BIBUF", "BIBUF", "BIBUF",
		"\t\t(cell BIBUF (cellType GENERIC) (view BIBUF (viewType NETLIST)"
				" (interface (port A (direction INPUT)) (port PAD (direction OUTPUT))"
				" (property MODEL (string \"OBUF\")))))\n",
		4,
		(char *[]){ "PAD", "A", "Q", "OE" }
	}, {
		"MUX", "MUX2", "MUX2",
		"\t\t(cell MUX2 (cellType GENERIC) (view MUX2 (viewType NETLIST)"
				" (interface (port D0 (direction INPUT)) (port D1 (direction INPUT)) (port S (direction INPUT)) (port Q (direction OUTPUT))"
				" (property MODEL (string \"MUX\")))))\n",
		4,
		(char *[]){ "Q", "D0", "D1", "S" }
	}, {
		"DFF", "FD", "FD",
		"\t\t(cell FD (cellType GENERIC) (view FD (viewType NETLIST)"
				" (interface\n"
				"\t\t\t(port D (direction INPUT))\n"
				"\t\t\t(port CLK (direction INPUT))\n"
				"\t\t\t(port Q (direction OUTPUT))\n"
				"\t\t\t(property MODEL (string \"DFF\")))\n"
				"\t\t\t))\n",
		3,
		(char *[]){ "Q", "D", "CLK" }
	}, {
		"ADDC", "FA", "FA",
		"\t\t(cell FA (cellType GENERIC) (view FA (viewType NETLIST)"
				" (interface\n"
				"\t\t\t(port A (direction INPUT))\n"
				"\t\t\t(port B (direction INPUT))\n"
				"\t\t\t(port CI (direction INPUT))\n"
				"\t\t\t(port SUM (direction OUTPUT))\n"
				"\t\t\t(port COUT (direction OUTPUT))\n"
				"\t\t\t(property MODEL (string \"ADDC\")))\n"
				"\t\t\t))\n",
		5,
		(char *[]){ "SUM", "COUT", "A", "B", "CI" }
	}, {
		NULL,
		NULL,
		NULL,
		NULL,
		0,
		NULL
	}
};
