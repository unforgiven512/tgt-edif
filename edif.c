/*
 * Copyright (c) 2001 Vladimir Dergachev (volodya@users.sourceforge.net)
 *    
 * Code skeleton derived from tgt-stub by Stephen Williams (steve@icarus.com)
 *
 *    This source code is free software;you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation;either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY;without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program;if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: edif.c,v 1.12 2001/07/11 01:52:30 volodya Exp $"
#endif

/*
 *  This is an edif output module.
 */

#include "ivl_target.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "string_cache.h"

static FILE *out=NULL;
STRING_CACHE *cells=NULL;

typedef void (*nexus_if)(ivl_scope_t, STRING_CACHE *, ivl_nexus_t);

typedef struct {
	char *key;
	char *cellref;
	char *viewref;
	char *description;
	long ports;
	char **port_name;
	} library_cell;

#define TAKEN (void *)0x0000001

#include "atmel_at40k.inc"

library_cell *current_library=atmel_at40k_library;

void iterate_scope_over_nexuses(ivl_scope_t current_scope, STRING_CACHE *nexuses, ivl_scope_t scope, nexus_if f);
void show_scope_instances(ivl_scope_t current_scope, STRING_CACHE *nexuses, ivl_scope_t scope);

long find_library_cell(char *key)
{
long i;
for(i=0;current_library[i].key!=NULL;i++){
	if(!strcmp(current_library[i].key,key))return i;
	}
fprintf(stderr,"Could not find library cell for key \"%s\"\n", key);
return -1;
}

long find_logic_cell(long logic_type)
{
char *key;
switch(logic_type){
	case IVL_LO_AND:
		key="AND";
		break;
	case IVL_LO_BUF:
		key="BUF";
		break;
	case IVL_LO_BUFIF0:
		key="BUFIF0";
		break;
	case IVL_LO_BUFIF1:
		key="BUFIF1";
		break;
	case IVL_LO_BUFZ:
		key="BUFZ";
		break;
	case IVL_LO_NAND:
		key="NAND";
		break;
	case IVL_LO_NOR:
		key="NOR";
		break;
	case IVL_LO_NOT:
		key="NOT";
		break;
	case IVL_LO_OR:
		key="OR";
		break;
	case IVL_LO_XOR:
		key="XOR";
		break;
	case IVL_LO_PULLDOWN:
		key="ZERO";
		break;
	case IVL_LO_PULLUP:
		key="ONE";
		break;
	default:
		fprintf(stderr,"Unsupported gate of type %d encountered\n", logic_type); 
		return -1;
	}
return find_library_cell(key);
}

long find_pad_cell(ivl_signal_t net)
{
char *pad;
long i_p=-1;
if((pad=(char *)ivl_signal_attr(net, "PAD"))!=NULL){
         switch(pad[0]){
                case 'i':
                         i_p=find_library_cell("IBUF");
                         break;
                 case 'o':
                         i_p=find_library_cell("OBUF");
                         break;
                 case 'b':
                         i_p=find_library_cell("BIBUF");
                         break;
                 default:
                         i_p=-1;
                 }
	}
return i_p;
}

/*
 * This function takes a signal name and mangles it into an equivalent
 * name that is suitable to the edif format.
 */
#define DIVISOR ((1<<31)-1)  /* it's prime */
#define MULTIPLE (27823485)

unsigned long strhash(const char * str)
{
long i;
unsigned long res;
res=str[0];
for(i=1;str[i];i++)res=(res*MULTIPLE+str[i])%DIVISOR;
return res;
}


char *mangle_buf=NULL;
long mangle_buf_size=0;
/* Beware ! I traded off convenience for simplicity (static buffer)
   mangle_edif_name() should not be called more then once as an argument 
   to a function and should not be called until you are done using 
   a buffer. If you really need to work with several mangled names simultaneously
   create several char * variables and strdup mangle return values into them.
*/
char *mangle_edif_name(const char *name)
{
long i;
int extra_mangle=0;

if((strlen(name)*2+20)>=mangle_buf_size){
	if(mangle_buf!=NULL)free(mangle_buf);
	mangle_buf_size=(2*strlen(name)+20);
	mangle_buf=calloc(mangle_buf_size, 1);
	}
for(i=0;name[i];i++){
	if((name[i]>='a') && (name[i]<='z')){
		mangle_buf[i]=name[i]+('A'-'a');
		extra_mangle=1;
		continue;
		}
	if((name[i]>='A') && (name[i]<='Z')){
		mangle_buf[i]=name[i];
		continue;
		}
	if((name[i]>='0') && (name[i]<='9')){
		mangle_buf[i]=name[i];
		continue;
		}
	mangle_buf[i]='_';
	if(name[i]!='_')extra_mangle=1;
	}
if(extra_mangle)
	sprintf(mangle_buf+i, "_%08lX", strhash(name));
	else mangle_buf[i]=0;
return mangle_buf;
}

int immediate_child(const char *parent, const char *child)
{
long i;
for(i=0;parent[i] && child[i];i++)
	if(parent[i]!=child[i])return -1;
if(!child[i])return -1;
if(child[i]!='.')return -1;
i++;
for(;child[i];i++)
	if(child[i]=='.')return 1;
return 0;
}

void start_net(ivl_nexus_t nex)
{
fprintf(out, "\t\t\t\t(net (rename %s \"%s\") (joined\n", 
     		mangle_edif_name(ivl_nexus_name(nex)),
		ivl_nexus_name(nex));
}

/* this defines iverilog version of standard logic cell using cells from
   vendor specific library
*/

/* all generated cell names are in the form of 

    IVERILOG_FUNCTION_size0_size1
    
    should never exceed 40 characters 
*/

char cell_name[40]; 

void define_logic_cell_header(char *name, long pins)
{
long i;
fprintf(out,"\t\t(cell %s (cellType GENERIC) (view %s (viewType NETLIST)\n",
	name, name);
fprintf(out,"\t\t\t(interface\n");
fprintf(out,"\t\t\t\t(port Q (direction OUTPUT))\n");
for(i=1;i<pins;i++)
	fprintf(out,"\t\t\t\t(port A%ld (direction INPUT))\n",i);
fprintf(out,"\t\t\t\t)\n"); /* ) of interface */
fprintf(out,"\t\t\t(contents\n");
}

long define_logic_cell(long type, long npins, STRING_CACHE *logic_cells)
{
long i, level_pins, rightmost_pin_level, level, cell;
switch(type){
	case IVL_LO_AND:
		/* 0's pin is always used for output */
		sprintf(cell_name, "IVERILOG_AND_%ld", npins-1);
		i=add_string(logic_cells, cell_name);
		if(logic_cells->data[i]!=NULL)return 0;
		logic_cells->data[i]=TAKEN;
		define_logic_cell_header(cell_name, npins);
		level_pins=npins-1;
		level=0;
		rightmost_pin_level=0;
		cell=find_library_cell("AND");
		if(cell<0){
			fprintf(stderr,"Could not find definition for AND2 cell, please update your vendor specific library\n");
			return -1;
			}
		do{
			for(i=0;i<level_pins/2;i++)
				fprintf(out,"\t\t\t\t(instance AND_%ld_%ld (viewRef %s (cellRef %s)))\n",
					level, i,
					current_library[cell].viewref, current_library[cell].cellref);
			for(i=0;i<level_pins/2;i++){
				fprintf(out,"\t\t\t\t(net NET_AND_%ld_%ld_A (joined\n", level, i);
				if(level){
					fprintf(out,"\t\t\t\t\t(portRef %s (instanceRef AND_%ld_%ld))\n",
						current_library[cell].port_name[0],
						level-1, 2*i);
					fprintf(out,"\t\t\t\t\t(portRef %s (instanceRef AND_%ld_%ld))\n",
						current_library[cell].port_name[1],
						level, i);
					} else {
					fprintf(out,"\t\t\t\t\t(portRef A%ld)\n", 2*i+1);
					fprintf(out,"\t\t\t\t\t(portRef %s (instanceRef AND_%ld_%ld))\n",
						current_library[cell].port_name[1],
						level, i);
					}
				fprintf(out,"\t\t\t\t\t))\n"); /* )) of net and joined */

				fprintf(out,"\t\t\t\t(net NET_AND_%ld_%ld_B (joined\n", level, i);
				if(level){
					if(i<(level_pins/2))
						fprintf(out,"\t\t\t\t\t(portRef %s (instanceRef AND_%ld_%ld))\n",
							current_library[cell].port_name[0],
							level-1, 2*i+1);
						else
						fprintf(out,"\t\t\t\t\t(portRef %s (instanceRef AND_%ld_%ld))\n",
							current_library[cell].port_name[0],
							rightmost_pin_level, 2*i+1);
					fprintf(out,"\t\t\t\t\t(portRef %s (instanceRef AND_%ld_%ld))\n",
						current_library[cell].port_name[2],
						level, i);
					} else {
					fprintf(out,"\t\t\t\t\t(portRef A%ld)\n", 2*i+2);
					fprintf(out,"\t\t\t\t\t(portRef %s (instanceRef AND_%ld_%ld))\n",
						current_library[cell].port_name[2],
						level, i);
					}
				fprintf(out,"\t\t\t\t\t))\n"); /* )) of net and joined */
				}
			if((level_pins & 1)==0)rightmost_pin_level=level;
			level_pins=(level_pins/2)+(level_pins & 1);
			level++;
			}while(level_pins>1);
		/* tie the output of the last AND gate into output */
		fprintf(out,"\t\t\t\t(net NET_AND_Q (joined\n");
		fprintf(out,"\t\t\t\t\t(portRef %s (instanceRef AND_%ld_0))\n",
			current_library[cell].port_name[0], level-1);
		fprintf(out,"\t\t\t\t\t(portRef Q)\n");
		fprintf(out,"\t\t\t\t\t))\n"); /* )) of net and joined */
		fprintf(out,"\t\t\t\t)\n"); /* ) of contents */
		fprintf(out,"\t\t\t))\n");  /* of view and cell */
		break;
	case IVL_LO_BUF:
		/* no need */
		break;
	case IVL_LO_BUFIF0:
		/* no need */
		break;
	case IVL_LO_BUFIF1:
		/* no need */
		break;
	case IVL_LO_BUFZ:
		/* no need */
		break;
	case IVL_LO_NAND:
		define_logic_cell(IVL_LO_AND, npins, logic_cells);
		sprintf(cell_name, "IVERILOG_NAND_%ld", npins-1);
		i=add_string(logic_cells, cell_name);
		if(logic_cells->data[i]!=NULL)return 0;
		logic_cells->data[i]=TAKEN;
		define_logic_cell_header(cell_name, npins);
		cell=find_library_cell("NOT");
		if(cell<0){
			fprintf(stderr,"Could not find cell NOT in vendor specific library\n");
			return -1;
			}
		fprintf(out,"\t\t\t\t(instance AND (viewRef IVERILOG_AND_%ld (cellRef IVERILOG_AND_%ld)))\n",
			npins-1, npins-1);
		fprintf(out,"\t\t\t\t(instance NOT (viewRef %s (cellRef %s)))\n",
			current_library[cell].viewref, current_library[cell].cellref);
		for(i=1;i<npins;i++)
			fprintf(out,"\t\t\t\t(net NET_A%ld (joined (portRef A%ld) (portRef A%ld (instanceRef AND))))\n",
				i,i,i);
		fprintf(out,"\t\t\t\t(net NET_Q0 (joined (portRef Q (instanceRef AND)) (portRef %s (instanceRef NOT))))\n",
			current_library[cell].port_name[1]);
		fprintf(out,"\t\t\t\t(net NET_Q1 (joined (portRef %s (instanceRef NOT)) (portRef Q)))\n",
			current_library[cell].port_name[0]);
		fprintf(out,"\t\t\t\t)\n"); /* ) of contents */
		fprintf(out,"\t\t\t))\n");  /* of view and cell */
		break;
	case IVL_LO_NOR:
		define_logic_cell(IVL_LO_OR, npins, logic_cells);
		sprintf(cell_name, "IVERILOG_NOR_%ld", npins-1);
		i=add_string(logic_cells, cell_name);
		if(logic_cells->data[i]!=NULL)return 0;
		logic_cells->data[i]=TAKEN;
		define_logic_cell_header(cell_name, npins);
		cell=find_library_cell("NOT");
		if(cell<0){
			fprintf(stderr,"Could not find cell NOT in vendor specific library\n");
			return -1;
			}
		fprintf(out,"\t\t\t\t(instance OR (viewRef IVERILOG_OR_%ld (cellRef IVERILOG_OR_%ld)))\n",
			npins-1, npins-1);
		fprintf(out,"\t\t\t\t(instance NOT (viewRef %s (cellRef %s)))\n",
			current_library[cell].viewref, current_library[cell].cellref);
		for(i=1;i<npins;i++)
			fprintf(out,"\t\t\t\t(net NET_A%ld (joined (portRef A%ld) (portRef A%ld (instanceRef OR))))\n",
				i,i,i);
		fprintf(out,"\t\t\t\t(net NET_Q0 (joined (portRef Q (instanceRef OR)) (portRef %s (instanceRef NOT))))\n",
			current_library[cell].port_name[1]);
		fprintf(out,"\t\t\t\t(net NET_Q1 (joined (portRef %s (instanceRef NOT)) (portRef Q)))\n",
			current_library[cell].port_name[0]);
		fprintf(out,"\t\t\t\t)\n"); /* ) of contents */
		fprintf(out,"\t\t\t))\n");  /* of view and cell */
		break;
	case IVL_LO_NOT:
		/* no need */
		break;
	case IVL_LO_OR:
		sprintf(cell_name, "IVERILOG_OR_%ld", npins-1);
		i=add_string(logic_cells, cell_name);
		if(logic_cells->data[i]!=NULL)return 0;
		logic_cells->data[i]=TAKEN;
		define_logic_cell_header(cell_name, npins);
		level_pins=npins-1;
		level=0;
		rightmost_pin_level=0;
		cell=find_library_cell("OR");
		if(cell<0){
			fprintf(stderr,"Could not find definition for OR2 cell, please update your vendor specific library\n");
			return -1;
			}
		do{
			for(i=0;i<level_pins/2;i++)
				fprintf(out,"\t\t\t\t(instance OR_%ld_%ld (viewRef %s (cellRef %s)))\n",
					level, i,
					current_library[cell].viewref, current_library[cell].cellref);
			for(i=0;i<level_pins/2;i++){
				fprintf(out,"\t\t\t\t(net NET_OR_%ld_%ld_A (joined\n", level, i);
				if(level){
					fprintf(out,"\t\t\t\t\t(portRef %s (instanceRef OR_%ld_%ld))\n",
						current_library[cell].port_name[0],
						level-1, 2*i);
					fprintf(out,"\t\t\t\t\t(portRef %s (instanceRef OR_%ld_%ld))\n",
						current_library[cell].port_name[1],
						level, i);
					} else {
					fprintf(out,"\t\t\t\t\t(portRef A%ld)\n", 2*i+1);
					fprintf(out,"\t\t\t\t\t(portRef %s (instanceRef OR_%ld_%ld))\n",
						current_library[cell].port_name[1],
						level, i);
					}
				fprintf(out,"\t\t\t\t\t))\n"); /* )) of net and joined */

				fprintf(out,"\t\t\t\t(net NET_OR_%ld_%ld_B (joined\n", level, i);
				if(level){
					if(i<(level_pins/2))
						fprintf(out,"\t\t\t\t\t(portRef %s (instanceRef OR_%ld_%ld))\n",
							current_library[cell].port_name[0],
							level-1, 2*i+1);
						else
						fprintf(out,"\t\t\t\t\t(portRef %s (instanceRef OR_%ld_%ld))\n",
							current_library[cell].port_name[0],
							rightmost_pin_level, 2*i+1);
					fprintf(out,"\t\t\t\t\t(portRef %s (instanceRef OR_%ld_%ld))\n",
						current_library[cell].port_name[2],
						level, i);
					} else {
					fprintf(out,"\t\t\t\t\t(portRef A%ld)\n", 2*i+2);
					fprintf(out,"\t\t\t\t\t(portRef %s (instanceRef OR_%ld_%ld))\n",
						current_library[cell].port_name[2],
						level, i);
					}
				fprintf(out,"\t\t\t\t\t))\n"); /* )) of net and joined */
				}
			if((level_pins & 1)==0)rightmost_pin_level=level;
			level_pins=(level_pins/2)+(level_pins & 1);
			level++;
			}while(level_pins>1);
		/* tie the output of the last OR gate into output */
		fprintf(out,"\t\t\t\t(net NET_OR_Q (joined\n");
		fprintf(out,"\t\t\t\t\t(portRef %s (instanceRef OR_%ld_0))\n",
			current_library[cell].port_name[0], level-1);
		fprintf(out,"\t\t\t\t\t(portRef Q)\n");
		fprintf(out,"\t\t\t\t\t))\n"); /* )) of net and joined */
		fprintf(out,"\t\t\t\t)\n"); /* ) of contents */
		fprintf(out,"\t\t\t))\n");  /* of view and cell */
		break;
	case IVL_LO_XOR:
		sprintf(cell_name, "IVERILOG_XOR_%ld", npins-1);
		i=add_string(logic_cells, cell_name);
		if(logic_cells->data[i]!=NULL)return 0;
		logic_cells->data[i]=TAKEN;
		define_logic_cell_header(cell_name, npins);
		level_pins=npins-1;
		level=0;
		rightmost_pin_level=0;
		cell=find_library_cell("XOR");
		if(cell<0){
			fprintf(stderr,"Could not find definition for XOR2 cell, please update your vendor specific library\n");
			return -1;
			}
		do{
			for(i=0;i<level_pins/2;i++)
				fprintf(out,"\t\t\t\t(instance XOR_%ld_%ld (viewRef %s (cellRef %s)))\n",
					level, i,
					current_library[cell].viewref, current_library[cell].cellref);
			for(i=0;i<level_pins/2;i++){
				fprintf(out,"\t\t\t\t(net NET_XOR_%ld_%ld_A (joined\n", level, i);
				if(level){
					fprintf(out,"\t\t\t\t\t(portRef %s (instanceRef XOR_%ld_%ld))\n",
						current_library[cell].port_name[0],
						level-1, 2*i);
					fprintf(out,"\t\t\t\t\t(portRef %s (instanceRef XOR_%ld_%ld))\n",
						current_library[cell].port_name[1],
						level, i);
					} else {
					fprintf(out,"\t\t\t\t\t(portRef A%ld)\n", 2*i+1);
					fprintf(out,"\t\t\t\t\t(portRef %s (instanceRef XOR_%ld_%ld))\n",
						current_library[cell].port_name[1],
						level, i);
					}
				fprintf(out,"\t\t\t\t\t))\n"); /* )) of net and joined */

				fprintf(out,"\t\t\t\t(net NET_XOR_%ld_%ld_B (joined\n", level, i);
				if(level){
					if(i<(level_pins/2))
						fprintf(out,"\t\t\t\t\t(portRef %s (instanceRef XOR_%ld_%ld))\n",
							current_library[cell].port_name[0],
							level-1, 2*i+1);
						else
						fprintf(out,"\t\t\t\t\t(portRef %s (instanceRef XOR_%ld_%ld))\n",
							current_library[cell].port_name[0],
							rightmost_pin_level, 2*i+1);
					fprintf(out,"\t\t\t\t\t(portRef %s (instanceRef XOR_%ld_%ld))\n",
						current_library[cell].port_name[2],
						level, i);
					} else {
					fprintf(out,"\t\t\t\t\t(portRef A%ld)\n", 2*i+2);
					fprintf(out,"\t\t\t\t\t(portRef %s (instanceRef XOR_%ld_%ld))\n",
						current_library[cell].port_name[2],
						level, i);
					}
				fprintf(out,"\t\t\t\t\t))\n"); /* )) of net and joined */
				}
			if((level_pins & 1)==0)rightmost_pin_level=level;
			level_pins=(level_pins/2)+(level_pins & 1);
			level++;
			}while(level_pins>1);
		/* tie the output of the last XOR gate into output */
		fprintf(out,"\t\t\t\t(net NET_XOR_Q (joined\n");
		fprintf(out,"\t\t\t\t\t(portRef %s (instanceRef XOR_%ld_0))\n",
			current_library[cell].port_name[0], level-1);
		fprintf(out,"\t\t\t\t\t(portRef Q)\n");
		fprintf(out,"\t\t\t\t\t))\n"); /* )) of net and joined */
		fprintf(out,"\t\t\t\t)\n"); /* ) of contents */
		fprintf(out,"\t\t\t))\n");  /* of view and cell */
		break;
	case IVL_LO_PULLDOWN:
		/* no need */
		break;
	case IVL_LO_PULLUP:
		/* no need */
		break;
	default:
		return -1;
	}
return 0;
}

void show_logic_cell_as_portref(ivl_net_logic_t log, long pin)
{
long i;
switch(ivl_logic_type(log)){
	case IVL_LO_AND:
	case IVL_LO_NAND:
	case IVL_LO_NOR:
	case IVL_LO_OR:
	case IVL_LO_XOR:
		if(pin)
			fprintf(out,"\t\t\t\t\t(portRef A%ld (instanceRef %s))\n", 
				pin, mangle_edif_name(ivl_logic_name(log)));
			else
			fprintf(out,"\t\t\t\t\t(portRef Q (instanceRef %s))\n", 
				mangle_edif_name(ivl_logic_name(log)));
		break;
	case IVL_LO_BUF:
	case IVL_LO_BUFIF0:
	case IVL_LO_BUFIF1:
	case IVL_LO_BUFZ:
	case IVL_LO_NOT:
	case IVL_LO_PULLDOWN:
	case IVL_LO_PULLUP:
		i=find_logic_cell(ivl_logic_type(log));
		if(i>=0)
			fprintf(out, "\t\t\t\t\t(portRef %s (instanceRef %s))\n", 
				current_library[i].port_name[pin], mangle_edif_name(ivl_logic_name(log)));
		        else 
			fprintf(stderr,"Unrecognized logic cell %s (type %d)\n",
			 	ivl_logic_name(log), ivl_logic_type(log));
			
	default:
	}
}

static void show_logic(ivl_scope_t current_scope, STRING_CACHE *nexuses, ivl_net_logic_t net)
{
long library_cell;
char *cell_kind;
char *name=(char *)ivl_logic_name(net);

switch(ivl_logic_type(net)){
	case IVL_LO_AND:
		cell_kind="AND";
		break;
	case IVL_LO_NAND:
		cell_kind="NAND";
		break;
	case IVL_LO_NOR:
		cell_kind="NOR";
		break;
	case IVL_LO_OR:
		cell_kind="OR";
		break;
	case IVL_LO_XOR:
		cell_kind="XOR";
		break;
	case IVL_LO_BUF:
	case IVL_LO_BUFIF0:
	case IVL_LO_BUFIF1:
	case IVL_LO_BUFZ:
	case IVL_LO_NOT:
	case IVL_LO_PULLDOWN:
	case IVL_LO_PULLUP:
		library_cell=find_logic_cell(ivl_logic_type(net));
		if(library_cell<0){
			fprintf(stderr,"Standard cell %s is not present in the library, ignoring\n", ivl_logic_name(net));
			return;
			}
		fprintf(out, "\t\t\t\t(instance (rename %s \"%s\")\n",mangle_edif_name(name), name);
		fprintf(out, "\t\t\t\t\t(viewRef %s (cellRef %s))\n", current_library[library_cell].viewref, current_library[library_cell].cellref );
		fprintf(out, "\t\t\t\t\t)\n"); /* ) of instance */
		return;
	default:
		return;
	}
fprintf(out, "\t\t\t\t(instance (rename %s \"%s\")\n",mangle_edif_name(name), name);
fprintf(out, "\t\t\t\t\t(viewRef IVERILOG_%s_%d (cellRef IVERILOG_%s_%d))\n", 
	cell_kind, ivl_logic_pins(net)-1, cell_kind, ivl_logic_pins(net)-1);
fprintf(out, "\t\t\t\t\t)\n"); /* ) of instance */
}


static void show_lpm_as_portref(ivl_scope_t current_scope, ivl_nexus_t nex, ivl_lpm_t lpm, long *portref_count)
{
long i;
long cell,cell_bufz;
long width=ivl_lpm_width(lpm);

cell_bufz=find_library_cell("BUFZ");
if(cell_bufz<0){
	fprintf(stderr,"Unimplemented library cell BUFZ. Update your vendor specific library.\n");
	return;
	}

switch(ivl_lpm_type(lpm)){
	case IVL_LPM_ADD:
		cell=find_library_cell("ADDC");
		if(cell<0){
			fprintf(stderr,"Unimplemented library cell ADDC. Update vendor specific library\n");
			return;
			}
		for(i=0;i<width;i++){
			if(nex==ivl_lpm_data(lpm, i)){
				if(!(*portref_count))start_net(nex);
				(*portref_count)++;
				if(width>1)
					fprintf(out,"\t\t\t\t\t(portRef %s (instanceRef (member %s %ld)))\n",
						current_library[cell].port_name[2],
						mangle_edif_name(ivl_lpm_name(lpm)),i);
					else
					fprintf(out,"\t\t\t\t\t(portRef %s (instanceRef %s))\n",
						current_library[cell].port_name[2],
						mangle_edif_name(ivl_lpm_name(lpm)));
				} else
			if(nex==ivl_lpm_datab(lpm, i)){
				if(!(*portref_count))start_net(nex);
				(*portref_count)++;
				if(width>1)
					fprintf(out,"\t\t\t\t\t(portRef %s (instanceRef (member %s %ld)))\n",
						current_library[cell].port_name[3],
						mangle_edif_name(ivl_lpm_name(lpm)),i);
					else
					fprintf(out,"\t\t\t\t\t(portRef %s (instanceRef %s))\n",
						current_library[cell].port_name[3],
						mangle_edif_name(ivl_lpm_name(lpm)));
				} else
			if(nex==ivl_lpm_q(lpm, i)){
				if(!(*portref_count))start_net(nex);
				(*portref_count)++;
				if(width>1)
					fprintf(out,"\t\t\t\t\t(portRef %s (instanceRef (member %s %ld)))\n",
						current_library[cell].port_name[0],
						mangle_edif_name(ivl_lpm_name(lpm)),i);
					else
					fprintf(out,"\t\t\t\t\t(portRef %s (instanceRef %s))\n",
						current_library[cell].port_name[0],
						mangle_edif_name(ivl_lpm_name(lpm)));
				}
			}
		break;
	case IVL_LPM_FF:
		cell=find_library_cell("DFF");
		if(cell<0){
			fprintf(stderr,"Unimplemented library cell DFF. Update vendor specific library\n");
			return;
			}
		for(i=0;i<width;i++){
			if(nex==ivl_lpm_data(lpm, i)){
				if(!(*portref_count))start_net(nex);
				(*portref_count)++;
				if(width>1)
					fprintf(out,"\t\t\t\t\t(portRef %s (instanceRef (member %s %ld)))\n",
						current_library[cell].port_name[1],
						mangle_edif_name(ivl_lpm_name(lpm)),i);
					else
					fprintf(out,"\t\t\t\t\t(portRef %s (instanceRef %s))\n",
						current_library[cell].port_name[1],
						mangle_edif_name(ivl_lpm_name(lpm)));
				} else
			if(nex==ivl_lpm_clk(lpm)){
				if(!(*portref_count))start_net(nex);
				(*portref_count)++;
				if(width>1)				
					fprintf(out,"\t\t\t\t\t(portRef %s (instanceRef (member %s %ld)))\n",
						current_library[cell].port_name[2],
						mangle_edif_name(ivl_lpm_name(lpm)),i);
					else
					fprintf(out,"\t\t\t\t\t(portRef %s (instanceRef %s))\n",
						current_library[cell].port_name[2],
						mangle_edif_name(ivl_lpm_name(lpm)));
				} else
			if(nex==ivl_lpm_q(lpm, i)){
				if(!(*portref_count))start_net(nex);
				(*portref_count)++;
				if(width>1)
					fprintf(out,"\t\t\t\t\t(portRef %s (instanceRef (member %s %ld)))\n",
						current_library[cell].port_name[0],
						mangle_edif_name(ivl_lpm_name(lpm)),i);
					else
					fprintf(out,"\t\t\t\t\t(portRef %s (instanceRef %s))\n",
						current_library[cell].port_name[0],
						mangle_edif_name(ivl_lpm_name(lpm)));
				} 
			}
		break;
	case IVL_LPM_MUX:
		cell=find_library_cell("MUX");
		if(cell<0){
			fprintf(stderr,"Unimplemented library cell MUX2. Update vendor specific library\n");
			return;
			}
		if(ivl_lpm_selects(lpm)!=1){
			fprintf(stderr,"LPM Multiplexors of more than 2 data are not implemented\n");
			return;
			}
		for(i=0;i<width;i++){
			if(nex==ivl_lpm_data2(lpm, 0, i)){
				if(!(*portref_count))start_net(nex);
				(*portref_count)++;
				if(width>1)
					fprintf(out,"\t\t\t\t\t(portRef %s (instanceRef (member %s %ld)))\n",
						current_library[cell].port_name[1],
						mangle_edif_name(ivl_lpm_name(lpm)),i);
					else
					fprintf(out,"\t\t\t\t\t(portRef %s (instanceRef %s))\n",
						current_library[cell].port_name[1],
						mangle_edif_name(ivl_lpm_name(lpm)));
				} else
			if(nex==ivl_lpm_data2(lpm, 1, i)){
				if(!(*portref_count))start_net(nex);
				(*portref_count)++;
				if(width>1)
					fprintf(out,"\t\t\t\t\t(portRef %s (instanceRef (member %s %ld)))\n",
						current_library[cell].port_name[2],
						mangle_edif_name(ivl_lpm_name(lpm)),i);
					else
					fprintf(out,"\t\t\t\t\t(portRef %s (instanceRef %s))\n",
						current_library[cell].port_name[2],
						mangle_edif_name(ivl_lpm_name(lpm)));
				} else
			if(nex==ivl_lpm_q(lpm, i)){
				if(!(*portref_count))start_net(nex);
				(*portref_count)++;
				if(width>1)
					fprintf(out,"\t\t\t\t\t(portRef %s (instanceRef (member %s %ld)))\n",
						current_library[cell].port_name[0],
						mangle_edif_name(ivl_lpm_name(lpm)),i);
					else
					fprintf(out,"\t\t\t\t\t(portRef %s (instanceRef %s))\n",
						current_library[cell].port_name[0],
						mangle_edif_name(ivl_lpm_name(lpm)));
				} else 
                        if(nex==ivl_lpm_select(lpm, 0)){
				if(!(*portref_count))start_net(nex);
				(*portref_count)++;
                                if(width>1)
                                        fprintf(out,"\t\t\t\t\t(portRef %s (instanceRef (member BUFZ_%s %ld)))\n",
						current_library[cell_bufz].port_name[1],
                                                mangle_edif_name(ivl_lpm_name(lpm)),i);
                                        else
                                        fprintf(out,"\t\t\t\t\t(portRef %s (instanceRef %s))\n",
						current_library[cell].port_name[3],
                                                mangle_edif_name(ivl_lpm_name(lpm)));
                                }
			}
		break;
	default:
		fprintf(stderr,"show_lpm_as_portref: Unimplemented lpm device %s\n", ivl_lpm_name(lpm));
	}
}

static void iterate_lpm_over_nexuses(ivl_scope_t current_scope, STRING_CACHE *nexuses, ivl_lpm_t lpm, nexus_if f)
{
long i;
long cell_bufz;
long width=ivl_lpm_width(lpm);

switch(ivl_lpm_type(lpm)){
	case IVL_LPM_ADD:
		for(i=0;i<width;i++){
			f(current_scope, nexuses, ivl_lpm_data(lpm, i));
			f(current_scope, nexuses, ivl_lpm_datab(lpm, i));
			f(current_scope, nexuses, ivl_lpm_q(lpm, i));
			}
		break;
	case IVL_LPM_FF:
		f(current_scope, nexuses, ivl_lpm_clk(lpm));
		for(i=0;i<width;i++){
			f(current_scope, nexuses, ivl_lpm_data(lpm, i));
			f(current_scope, nexuses, ivl_lpm_q(lpm, i));
			}
		break;
	case IVL_LPM_MUX:
		if(ivl_lpm_selects(lpm)!=1){
			fprintf(stderr,"LPM Multiplexors of more than 2 data are not implemented\n");
			return;
			}
                f(current_scope, nexuses, ivl_lpm_select(lpm, 0));
		for(i=0;i<width;i++){
			f(current_scope, nexuses, ivl_lpm_data2(lpm, 0, i));
			f(current_scope, nexuses, ivl_lpm_data2(lpm, 1, i));
			f(current_scope, nexuses, ivl_lpm_q(lpm, i));
			}
		break;
	default:
		fprintf(stderr,"show_lpm_as_portref: Unimplemented lpm device %s\n", ivl_lpm_name(lpm));
	}
}


static void show_lpm(ivl_scope_t current_scope, STRING_CACHE *nexuses, ivl_lpm_t net)
{
long idx,i,i_z,i_b;
long width=ivl_lpm_width(net);
char *mangled_name;

switch(ivl_lpm_type(net)){
	case IVL_LPM_ADD: 
		i=find_library_cell("ADDC");
		i_z=find_library_cell("ZERO");
		if(i<0)break;
		mangled_name=strdup(mangle_edif_name(ivl_lpm_name(net)));
		if(width>1)
			fprintf(out,"\t\t\t\t(instance (array (rename %s \"%s\") %ld)\n",
				mangled_name,
				ivl_lpm_name(net), width);
			else
			fprintf(out,"\t\t\t\t(instance (rename %s \"%s\")\n",
				mangled_name, 
				ivl_lpm_name(net));
		fprintf(out,"\t\t\t\t\t(viewRef %s (cellRef %s)))\n",
					current_library[i].viewref, current_library[i].cellref);
		fprintf(out,"\t\t\t\t(instance ZERO_%s (viewRef %s (cellRef %s)))\n",
					mangled_name,
					current_library[i_z].viewref, current_library[i_z].cellref);
		/* ground first carry in */
		fprintf(out,"\t\t\t\t(net NET_%s_0 (joined\n"
				"\t\t\t\t\t(portRef %s (instanceRef ZERO_%s))\n"
				"\t\t\t\t\t(portRef %s (instanceRef (member %s 0)))\n"
				"\t\t\t\t\t))\n",
				mangled_name, 
				current_library[i_z].port_name[0],
				mangled_name,
				current_library[i].port_name[4],
				mangled_name);
		/* connect carry outs with carry ins */
		for(idx=1;idx<width;idx++){
			fprintf(out,"\t\t\t\t(net NET_%s_%ld (joined\n",
					mangled_name, idx);
			fprintf(out,"\t\t\t\t\t(portRef %s (instanceRef (member %s %ld)))\n",
					current_library[i].port_name[4],mangled_name, idx);
			fprintf(out,"\t\t\t\t\t(portRef %s (instanceRef (member %s %ld)))\n",
					current_library[i].port_name[1], mangled_name, idx-1);
			fprintf(out,"\t\t\t\t\t))\n"); /* ) of net and joined*/
			}
		free(mangled_name);
		break;
	  

	case IVL_LPM_SUB: 
		fprintf(stderr,"IVL_LPM_SUB has not been implemented yet. Error in %s\n",
				ivl_lpm_name(net));
		break;
	  

	case IVL_LPM_FF: 
		i=find_library_cell("DFF");
		if(i<0)break;

		if(width>1)
			fprintf(out,"\t\t\t\t(instance (array (rename %s \"%s\") %ld)\n",
				mangle_edif_name(ivl_lpm_name(net)),
				ivl_lpm_name(net), width);
			else
			fprintf(out,"\t\t\t\t(instance (rename %s \"%s\")\n",
				mangle_edif_name(ivl_lpm_name(net)), 
				ivl_lpm_name(net));
		fprintf(out,"\t\t\t\t\t(viewRef %s (cellRef %s)))\n",
					current_library[i].viewref, current_library[i].cellref);
		break;
	  

	case IVL_LPM_MUX: 
		i=find_library_cell("MUX");
		if(i<0)break;
		i_b=find_library_cell("BUFZ");
		if(i_b<0)break;

		if(width>1){
			fprintf(out,"\t\t\t\t(instance (array (rename %s \"%s\") %ld)\n",
				mangle_edif_name(ivl_lpm_name(net)), 
				ivl_lpm_name(net), width);
                	fprintf(out,"\t\t\t\t\t(viewRef %s (cellRef %s)))\n",
                                        current_library[i].viewref, current_library[i].cellref);
			fprintf(out,"\t\t\t\t(instance (array (rename BUFZ_%s \"%s\") %ld)\n",
				mangle_edif_name(ivl_lpm_name(net)),
				ivl_lpm_name(net), width);
			fprintf(out,"\t\t\t\t\t(viewRef %s (cellRef %s)))\n",
				current_library[i_b].viewref, current_library[i_b].cellref);
			for(idx=0;idx<width;idx++)
				fprintf(out,"\t\t\t\t(net NET_%s_%ld (joined\n"
					    "\t\t\t\t\t(portRef %s (instanceRef (member %s %ld)))\n"
					    "\t\t\t\t\t(portRef %s (instanceRef (member BUFZ_%s %ld)))\n"
					    "\t\t\t\t\t))\n", 
					mangle_edif_name(ivl_lpm_name(net)), idx, 
					current_library[i].port_name[3], mangle_edif_name(ivl_lpm_name(net)), idx, 
					current_library[i_b].port_name[0], mangle_edif_name(ivl_lpm_name(net)), idx);
			} else {
		        fprintf(out,"\t\t\t\t(instance (rename %s \"%s\")\n",
				mangle_edif_name(ivl_lpm_name(net)), 
				ivl_lpm_name(net));
       	         	fprintf(out,"\t\t\t\t\t(viewRef %s (cellRef %s)))\n",
                                        current_library[i].viewref, current_library[i].cellref);

			}
		break;
	  

	default:
	    fprintf(stderr, "  Unhandled LPM primitive: %s: <width=%u>\n", ivl_lpm_name(net),
		    ivl_lpm_width(net));
	}
}

/* this function returns -1 if no match,
   0 if match
   and 1 if successor */

int compare_scope_names(const char *basename,const char *name,const char *fullname)
{
long i,j,k,m;
for(i=0;basename[i] && fullname[i];i++){
	if(basename[i]!=fullname[i])return -2;
	}
if((fullname[i]!='.')||(basename[i]))return -4;
k=strlen(name);
m=strlen(fullname);
for(j=1;(j<=k) && (j<=m);j++){
	if(name[k-j]!=fullname[m-j])return -3;
	}
if(fullname[m-j]!='.')return -1;
if((m-j)>i)return 1;
if((m-j)==i)return 0;
return -1;
}

void print_signal_scope_name(ivl_signal_t sig)
{
char *name;
long i,j;
name=strdup(ivl_signal_name(sig));
i=strlen(name);
j=strlen(ivl_signal_basename(sig));
if((j+1)<i){
	name[i-j-1]=0;
	fprintf(out,"%s", mangle_edif_name(name));
	}
free(name);
}

static void show_signal_as_interface(ivl_scope_t current_scope, ivl_signal_t sig)
{
char *pad;
if(ivl_signal_port(sig)!=IVL_SIP_NONE){
	if(ivl_signal_pins(sig)>1)
		fprintf(out, "\t\t\t\t(port (array (rename %s \"%s\") %d)\n", 
			mangle_edif_name(ivl_signal_basename(sig)),
			ivl_signal_basename(sig), ivl_signal_pins(sig));
		else 
		fprintf(out, "\t\t\t\t(port (rename %s \"%s\")\n", 
			mangle_edif_name(ivl_signal_basename(sig)),
			ivl_signal_basename(sig));
	switch(ivl_signal_port(sig)){
		case IVL_SIP_INPUT:
			fprintf(out, "\t\t\t\t\t(direction INPUT)\n");
			break;
		case IVL_SIP_OUTPUT:
			fprintf(out, "\t\t\t\t\t(direction OUTPUT)\n");
			break;
		case IVL_SIP_INOUT:
			fprintf(out, "\t\t\t\t\t(direction INOUT)\n");
			break;
		default:
		}						
	fprintf(out, "\t\t\t\t\t)\n"); /* ) of portRef */
	} else
if((pad=(char *)ivl_signal_attr(sig, "PAD"))!=NULL){
	fprintf(out, "\t\t\t\t(port (rename %s  \"%s\") (property PIN_LOCATION (string \"%s\")) ", 
			mangle_edif_name(pad), 
			pad,
			pad);
	switch(pad[0]){
		case 'i':
			fprintf(out,"(direction INPUT))\n");
			break;
		case 'o':
			fprintf(out,"(direction OUTPUT))\n");
			break;
		case 'b':
			fprintf(out,"(direction INOUT))\n");
			break;
		}					
	}
}

static void show_nexus_as_instance(ivl_scope_t current_scope, STRING_CACHE *nexuses, ivl_nexus_t nex)
{
long i,idx;
idx=add_string(nexuses, ivl_nexus_name(nex));
if(nexuses->data[idx]!=NULL)return;
nexuses->data[idx]=nex;
for (idx=0;idx<ivl_nexus_ptrs(nex);idx++){
     	ivl_net_const_t con;
        ivl_nexus_ptr_t ptr=ivl_nexus_ptr(nex, idx);
        if((con=ivl_nexus_ptr_con(ptr))!=NULL){
        	const char*bits=ivl_const_bits(con);
                unsigned pin=ivl_nexus_ptr_pin(ptr);
                if(bits[pin]=='1'){
			i=find_library_cell("ONE");
			if(i>=0)
                                fprintf(out,"\t\t\t\t(instance ONE_%s_%ld (viewRef %s (cellRef %s)))\n",
      		                                mangle_edif_name(ivl_nexus_name(nex)), idx,
					current_library[i].viewref, current_library[i].cellref);
                        } else {
			i=find_library_cell("ZERO");
			if(i>=0)
                                fprintf(out,"\t\t\t\t(instance ZERO_%s_%ld (viewRef %s (cellRef %s)))\n",
      	                                 	mangle_edif_name(ivl_nexus_name(nex)), idx,
					current_library[i].viewref, current_library[i].cellref);
                        }
                 }
	} 
}

static void show_signal_as_instance(ivl_scope_t current_scope, ivl_signal_t net)
{
long i_p;
char *pad;
if((i_p=find_pad_cell(net))>=0){
        pad=(char *)ivl_signal_attr(net, "PAD");
        fprintf(out,"\t\t\t\t(instance (rename %s \"%s\")\n",
                mangle_edif_name(pad),
                pad);
        fprintf(out,"\t\t\t\t\t(viewRef %s (cellRef %s))\n",
                        current_library[i_p].viewref, current_library[i_p].cellref);
        fprintf(out,"\t\t\t\t\t(property PinName (string \"%s\"))\n", pad+1);
        fprintf(out,"\t\t\t\t\t)\n"); /* ) for instance */
        fprintf(out,"\t\t\t\t(net (rename %s \"%s\") (joined\n",
                mangle_edif_name(pad), pad);
        fprintf(out,"\t\t\t\t\t(portRef %s)\n", mangle_edif_name(ivl_signal_attr(net, "PAD")));
        fprintf(out,"\t\t\t\t\t(portRef %s (instanceRef %s))\n",
                current_library[i_p].port_name[0],
                mangle_edif_name(pad));
        fprintf(out,"\t\t\t\t\t))\n"); /* ) of net and joined*/
        }
}

static void show_nexus(ivl_scope_t current_scope, STRING_CACHE *nexuses, ivl_nexus_t nex)
{
long idx, i, i_p;
long portref_count;

idx=add_string(nexuses, ivl_nexus_name(nex));
if(nexuses->data[idx]!=NULL)return;
nexuses->data[idx]=nex;

portref_count=0;

for (idx=0;idx<ivl_nexus_ptrs(nex);idx++){
	ivl_net_const_t con;
	ivl_net_logic_t log;
	ivl_lpm_t lpm;
	ivl_signal_t sig;
	ivl_nexus_ptr_t ptr=ivl_nexus_ptr(nex, idx);

	if((sig=ivl_nexus_ptr_sig(ptr))){
        	i=compare_scope_names(ivl_scope_name(current_scope),ivl_signal_basename(sig), ivl_signal_name(sig));  
                if(i>=0){
	  		if((i_p=find_pad_cell(sig))>=0){
				if(!portref_count)start_net(nex);
				portref_count++;
      	              		fprintf(out, "\t\t\t\t\t(portRef %s (instanceRef %s))\n",
             		              		current_library[i_p].port_name[1],
              		              		mangle_edif_name(ivl_signal_attr(sig, "PAD")));
				}
			
			if((i==0) && (ivl_signal_port(sig)!=IVL_SIP_NONE)){
				if(!portref_count)start_net(nex);
				portref_count++;
				if(ivl_signal_pins(sig)>1)
					fprintf(out, "\t\t\t\t\t(portRef (member %s %d)\n", 
			  			mangle_edif_name(ivl_signal_basename(sig)), ivl_nexus_ptr_pin(ptr));
					else
					fprintf(out, "\t\t\t\t\t(portRef %s\n", 
			  			mangle_edif_name(ivl_signal_basename(sig)));
				if(i>0){
					fprintf(out, "\t\t\t\t\t\t(instanceRef ");
					print_signal_scope_name(sig);
					fprintf(out,")\n");
					} else 
				if(ivl_signal_attr(sig, "PAD")!=NULL){
					fprintf(out, "\t\t\t\t\t\t(property PIN (string \"%s\"))\n", ivl_signal_attr(sig, "PAD"));
					}
				fprintf(out, "\t\t\t\t\t)\n"); /* ) of portRef */
				}
			}				
		 } else 
	if((log=ivl_nexus_ptr_log(ptr))!=NULL){
		if(!portref_count)start_net(nex);
		portref_count++;
		show_logic_cell_as_portref(log, ivl_nexus_ptr_pin(ptr));
	  	} else 
	if((lpm=ivl_nexus_ptr_lpm(ptr))!=NULL){
		show_lpm_as_portref(current_scope, nex, lpm, &portref_count);
		} else 
	if((con=ivl_nexus_ptr_con(ptr))!=NULL){
		const char*bits=ivl_const_bits(con);
		switch(bits[ivl_nexus_ptr_pin(ptr)]){
			case '1':
				i=find_library_cell("ONE");
				if(i>=0){
					if(!portref_count)start_net(nex);
					portref_count++;
					fprintf(out,"\t\t\t\t\t(portRef %s (instanceRef ONE_%s_%ld))\n",
						current_library[i].port_name[0], mangle_edif_name(ivl_nexus_name(nex)),idx);
					} else {
					fprintf(stderr,"Cannot handle consant value '%c': cell ONE has not been found in vendor specific library\n",
						bits[ivl_nexus_ptr_pin(ptr)]);
					}
				break;
			case '0':
				i=find_library_cell("ZERO");
				if(i>=0){
					if(!portref_count)start_net(nex);
					portref_count++;
					fprintf(out,"\t\t\t\t\t(portRef %s (instanceRef ZERO_%s_%ld))\n",
						current_library[i].port_name[0], mangle_edif_name(ivl_nexus_name(nex)),idx);
					} else {
					fprintf(stderr,"Cannot handle consant value '%c': cell ZERO has not been found in vendor specific library\n",
						bits[ivl_nexus_ptr_pin(ptr)]);
					}
				break;
			default:
				fprintf(stderr,"Cannot handle consant value '%c' while processing nexus %s\n",
					bits[ivl_nexus_ptr_pin(ptr)], ivl_nexus_name(nex));
				break;
			}
		} else {
		fprintf(stderr, " unrecognized nexus connection in nexus %s 0x%08x 0x%08x\n", ivl_nexus_name(nex), ((unsigned *)ptr)[0], ((unsigned *)ptr)[1]);
		}
	}
if(portref_count){
	fprintf(out,"\t\t\t\t)\n"); /* ) for  joined */
	fprintf(out,"\t\t\t\t)\n"); /* ) for net */
	}
}

static void output_standard_cells(void)
{
long i;
for(i=0;current_library[i].description!=NULL;i++){
	fprintf(out, current_library[i].description);
	}
}

static void iterate_logic_over_nexuses(ivl_scope_t current_scope, STRING_CACHE *nexuses, ivl_net_logic_t net, nexus_if f)
{
long npins, i;
fprintf(stderr,"show_logic_as_net: processing %s\n", ivl_logic_name(net));
npins=ivl_logic_pins(net);
for(i=0;i<npins;i++)
	f(current_scope, nexuses, ivl_logic_pin(net, i));
}

/* this function should really be in the main body */
char *ivl_scope_basename(ivl_scope_t net)
{
char *name;
long i;
name=(char *)ivl_scope_name(net);
i=strlen(name)-1;
while((i>=0)&&(name[i]!='.'))i--;
if(name[i]=='.')i++;
return name+i;
}


typedef struct {
	ivl_scope_t current_scope;
	STRING_CACHE *nexuses;
	nexus_if f;
	} ison_s;

static int ison_helper(ivl_scope_t scope, ison_s *s)
{
iterate_scope_over_nexuses(s->current_scope, s->nexuses, scope, s->f);
return 0;
}

void iterate_scope_over_nexuses(ivl_scope_t current_scope, STRING_CACHE *nexuses, ivl_scope_t scope, nexus_if f)
{
long idx,pin;
ison_s s;
ivl_nexus_t nex;
ivl_signal_t sig;
switch (ivl_scope_type(scope)){
	case IVL_SCT_MODULE:
		s.current_scope=current_scope;
		s.nexuses=nexuses;
		s.f=f;
		ivl_scope_children(scope, ison_helper, &s);
		for(idx=0;idx<ivl_scope_sigs(scope);idx++){
			sig=ivl_scope_sig(scope, idx);
			for(pin=0;pin<ivl_signal_pins(sig);pin++){
				nex=ivl_signal_pin(sig, pin);
				f(current_scope, nexuses, nex);
			      	}
			}
            	for (idx=0;idx<ivl_scope_logs(scope);idx++)
            	    	iterate_logic_over_nexuses(scope, nexuses, ivl_scope_log(scope, idx), f);
            	for (idx=0;idx<ivl_scope_lpms(scope);idx++)
	            	iterate_lpm_over_nexuses(scope, nexuses, ivl_scope_lpm(scope, idx), f);			
		break;
	default:
	}
}

static int isi_helper(ivl_scope_t scope, ison_s *s)
{
show_scope_instances(s->current_scope, s->nexuses, scope);
return 0;
}


static int definition_helper(ivl_scope_t scope, STRING_CACHE *cell_definitions)
{
long i;
for(i=0;i<ivl_scope_logs(scope);i++)
	define_logic_cell(ivl_logic_type(ivl_scope_log(scope, i)), ivl_logic_pins(ivl_scope_log(scope, i)), cell_definitions);
}

void show_scope_instances(ivl_scope_t current_scope, STRING_CACHE *nexuses, ivl_scope_t scope)
{
long idx;
ison_s s;
switch (ivl_scope_type(scope)){
	case IVL_SCT_MODULE:
		s.current_scope=current_scope;
		s.nexuses=nexuses;
		ivl_scope_children(scope, isi_helper, &s);
            	for (idx=0;idx<ivl_scope_logs(scope);idx++)
            	    	show_logic(scope, nexuses, ivl_scope_log(scope, idx));
            	for (idx=0;idx<ivl_scope_lpms(scope);idx++)
	            	show_lpm(scope, nexuses, ivl_scope_lpm(scope, idx));
		break;
	default:
	}
}

static int show_scope(ivl_scope_t net, void*x)
{
long idx;
STRING_CACHE *nexuses;
STRING_CACHE *cell_definitions;


switch(ivl_scope_type(net)){
	case IVL_SCT_MODULE:
	    	idx=add_string(cells, mangle_edif_name(ivl_scope_tname(net)));
	    	if(cells->data[idx]!=NULL)break;
	    	cells->data[idx]=net; /* mark this scope */
		cell_definitions=new_string_cache();
		ivl_scope_children(net, definition_helper, cell_definitions);
		free_string_cache(cell_definitions);
	    	fprintf(stderr, "Processing module: %s (%u signals, %u logic)\n",
	    	  	ivl_scope_tname(net), ivl_scope_sigs(net),
	      	  	ivl_scope_logs(net));

		fprintf(stderr,"show_scope: processing scope %s\n", ivl_scope_name(net));
	    	fprintf(out, "\t\t(cell (rename %s \"%s\") (cellType GENERIC)",
	    		mangle_edif_name(ivl_scope_tname(net)), ivl_scope_tname(net));
	    	fprintf(out, " (view %s (viewType NETLIST)\n", 
			mangle_edif_name(ivl_scope_tname(net)));
	    	fprintf(out, "\t\t\t(interface\n");
	    	for(idx=0;idx<ivl_scope_sigs(net);idx++)
	    	    	show_signal_as_interface(net, ivl_scope_sig(net, idx));
	    	/* generate interface */
	    	fprintf(out, "\t\t\t\t)\n"); /* ) of interface */
	    	fprintf(out, "\t\t\t(contents\n");
	    	/* generate contents here */
		/* instances first */
		nexuses=new_string_cache();
		/* this only needs to be done for toplevel module - no pads in lowlevel ones */
		for(idx=0;idx<ivl_scope_sigs(net);idx++)
			show_signal_as_instance(net, ivl_scope_sig(net, idx));

		show_scope_instances(net, nexuses, net);
		iterate_scope_over_nexuses(net, nexuses, net, show_nexus_as_instance);
		free_string_cache(nexuses);
		/* now handle nets */
		nexuses=new_string_cache();
		iterate_scope_over_nexuses(net, nexuses, net, show_nexus);
	    	fprintf(out, "\t\t\t\t)\n"); /* ) of contents */
	    	fprintf(out, "\t\t\t))\n"); /* ) of cell and view*/
		free_string_cache(nexuses);
	    	break;
	case IVL_SCT_FUNCTION:
	    	fprintf(stderr, " Unhandled function %s\n", ivl_scope_tname(net));
	    	break;
	case IVL_SCT_BEGIN:
	    	fprintf(stderr, " Unhandled begin : %s\n", ivl_scope_tname(net));
	    	break;
	case IVL_SCT_FORK:
	    	fprintf(stderr, " Unhandled fork : %s\n", ivl_scope_tname(net));
	    	break;
	case IVL_SCT_TASK:
	    	fprintf(stderr, " Unhandled task %s\n", ivl_scope_tname(net));
	    	break;
	default:
	    	fprintf(stderr, " Unhandled type(%u) %s\n", ivl_scope_type(net),
		    	ivl_scope_tname(net));
      	}
return 0;
}


int target_design(ivl_design_t des)
{
const char *path=ivl_design_flag(des, "-o");
time_t seconds;
struct tm *tm_time;
char *module_name;

if(path==0){
	return -1;
      	}

out=fopen(path, "w");
if(out==0){
	perror(path);
	return -2;
      	}
module_name=strdup(mangle_edif_name(ivl_scope_name(ivl_design_root(des))));
cells=new_string_cache();
seconds=time(NULL);
tm_time=localtime(&seconds);
fprintf(stderr, "Writing to %s\n" , path);
fprintf(out,"(edif (rename %s \"%s\")\n", module_name, ivl_scope_name(ivl_design_root(des)));
fprintf(out,"\t(edifVersion 2 0 0)\n"
 	    "\t(edifLevel 0)\n"
	    "\t(keywordMap (keywordLevel 0))\n"
	    "\t(status\n"
	    "\t\t(written\n"
	    "\t\t\t(timeStamp %d %d %d %d %d %d)\n", 
		tm_time->tm_year+1900,
		tm_time->tm_mon,
		tm_time->tm_mday,
		tm_time->tm_hour,
		tm_time->tm_min,
		tm_time->tm_sec);
fprintf(out,"\t\t\t(program \"iverilog\"\n"
      	    "\t\t\t\t(version \"iverilog %s\")\n"
	    "\t\t\t\t)\n", "alpha edif output module, version of Tue Jul 10 21:36:41 2001"); /* replace with real version string later */
fprintf(out,"\t\t\t)\n"); /* ) of written */		  
fprintf(out,"\t\t)\n"); /* ) of status */
      
fprintf(out,"\t(library %s (edifLevel 0) (technology (numberDefinition))\n", module_name);
output_standard_cells();
show_scope(ivl_design_root(des), NULL);
fprintf(out,"\t\t)\n"); /* ) of library */

fprintf(out,"\t(design (rename %s \"%s\")\n"
            "\t\t(cellRef %s (libraryRef %s)))\n", 
		  		module_name,
				ivl_scope_name(ivl_design_root(des)),
				module_name,
				module_name);
fprintf(out,"\t)\n"); /* ) of edif */
fclose(out);
free(module_name);
return 0;
}

#if  defined (__CYGWIN32__)
#include <cygwin/cygwin_dll.h>
DECLARE_CYGWIN_DLL(DllMain);
#endif

