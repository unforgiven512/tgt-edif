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
#ident "$Id: edif.c,v 1.3 2001/07/06 11:16:09 volodya Exp $"
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
STRING_CACHE *nexuses=NULL;

typedef struct {
	char *key;
	char *cellref;
	char *viewref;
	char *description;
	long ports;
	char **port_name;
	} library_cell;

#include "atmel_at40k.inc"

library_cell *current_library=atmel_at40k_library;

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
		return -1;
	}
return find_library_cell(key);
}

long find_lpm_cell(long lpm_type)
{
char *key;
switch(lpm_type){
	case IVL_LPM_ADD:
		key="LPM_ADD";
		break;
	case IVL_LPM_CMP_GE:
		key="LPM_CMP_GE";
		break;
	case IVL_LPM_CMP_GT:
		key="LPM_CMP_GT";
		break;
	case IVL_LPM_FF:
		key="LPM_FF";
		break;
	case IVL_LPM_MULT:
		key="LPM_MULT";
		break;
	case IVL_LPM_MUX:
		key="LPM_MUX";
		break;
	case IVL_LPM_SUB:
		key="LPM_SUB";
		break;
	case IVL_LPM_RAM:
		key="LPM_RAM";
		break;
	default:
		return -1;
	}
return find_library_cell(key);
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
	sprintf(mangle_buf+i, "_%lX", strhash(name));
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

static void show_lpm_as_portref(ivl_scope_t current_scope, ivl_nexus_t nex, ivl_lpm_t lpm)
{
long i;
long cell;
long width=ivl_lpm_width(lpm);

if(immediate_child(ivl_scope_name(current_scope), ivl_lpm_name(lpm))!=0)return;		

switch(ivl_lpm_type(lpm)){
	case IVL_LPM_ADD:
		cell=find_library_cell("ADDC");
		if(cell<0){
			fprintf(stderr,"Unimplemented library cell ADDC. Update vendor specific library\n");
			return;
			}
		for(i=0;i<width;i++){
			if(nex==ivl_lpm_data(lpm, i)){
				if(width>1)
					fprintf(out,"\t\t\t\t\t(portRef A (instanceRef (member %s %ld)))\n",
						mangle_edif_name(ivl_lpm_name(lpm)),i);
					else
					fprintf(out,"\t\t\t\t\t(portRef A (instanceRef %s))\n",
						mangle_edif_name(ivl_lpm_name(lpm)));
				} else
			if(nex==ivl_lpm_datab(lpm, i)){
				if(width>1)
					fprintf(out,"\t\t\t\t\t(portRef B (instanceRef (member %s %ld)))\n",
						mangle_edif_name(ivl_lpm_name(lpm)),i);
					else
					fprintf(out,"\t\t\t\t\t(portRef B (instanceRef %s))\n",
						mangle_edif_name(ivl_lpm_name(lpm)));
				} else
			if(nex==ivl_lpm_q(lpm, i)){
				if(width>1)
					fprintf(out,"\t\t\t\t\t(portRef SUM (instanceRef (member %s %ld)))\n",
						mangle_edif_name(ivl_lpm_name(lpm)),i);
					else
					fprintf(out,"\t\t\t\t\t(portRef SUM (instanceRef %s))\n",
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
				if(width>1)
					fprintf(out,"\t\t\t\t\t(portRef D (instanceRef (member %s %ld)))\n",
						mangle_edif_name(ivl_lpm_name(lpm)),i);
					else
					fprintf(out,"\t\t\t\t\t(portRef D (instanceRef %s))\n",
						mangle_edif_name(ivl_lpm_name(lpm)));
				} else
			if(nex==ivl_lpm_clk(lpm)){
				if(width>1)
					fprintf(out,"\t\t\t\t\t(portRef CLK (instanceRef (member %s %ld)))\n",
						mangle_edif_name(ivl_lpm_name(lpm)),i);
					else
					fprintf(out,"\t\t\t\t\t(portRef CLK (instanceRef %s))\n",
						mangle_edif_name(ivl_lpm_name(lpm)));
				} else
			if(nex==ivl_lpm_q(lpm, i)){
				if(width>1)
					fprintf(out,"\t\t\t\t\t(portRef Q (instanceRef (member %s %ld)))\n",
						mangle_edif_name(ivl_lpm_name(lpm)),i);
					else
					fprintf(out,"\t\t\t\t\t(portRef Q (instanceRef %s))\n",
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
				if(width>1)
					fprintf(out,"\t\t\t\t\t(portRef D0 (instanceRef (member %s %ld)))\n",
						mangle_edif_name(ivl_lpm_name(lpm)),i);
					else
					fprintf(out,"\t\t\t\t\t(portRef D0 (instanceRef %s))\n",
						mangle_edif_name(ivl_lpm_name(lpm)));
				} else
			if(nex==ivl_lpm_data2(lpm, 1, i)){
				if(width>1)
					fprintf(out,"\t\t\t\t\t(portRef D1 (instanceRef (member %s %ld)))\n",
						mangle_edif_name(ivl_lpm_name(lpm)),i);
					else
					fprintf(out,"\t\t\t\t\t(portRef D1 (instanceRef %s))\n",
						mangle_edif_name(ivl_lpm_name(lpm)));
				} else
			if(nex==ivl_lpm_q(lpm, i)){
				if(width>1)
					fprintf(out,"\t\t\t\t\t(portRef Q (instanceRef (member %s %ld)))\n",
						mangle_edif_name(ivl_lpm_name(lpm)),i);
					else
					fprintf(out,"\t\t\t\t\t(portRef Q (instanceRef %s))\n",
						mangle_edif_name(ivl_lpm_name(lpm)));
				} else 
                        if(nex==ivl_lpm_select(lpm, 0)){
                                if(width>1)
                                        fprintf(out,"\t\t\t\t\t(portRef A (instanceRef (member BUFZ_%s %ld)))\n",
                                                mangle_edif_name(ivl_lpm_name(lpm)),i);
                                        else
                                        fprintf(out,"\t\t\t\t\t(portRef S (instanceRef %s))\n",
                                                mangle_edif_name(ivl_lpm_name(lpm)));
                                }
			}
		break;
	default:
		fprintf(stderr,"show_lpm_as_portref: Unimplemented lpm device %s\n", ivl_lpm_name(lpm));
	}
}

static void show_lpm(ivl_lpm_t net)
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
				"\t\t\t\t\t(portRef Q (instanceRef ZERO_%s))\n"
				"\t\t\t\t\t(portRef CI (instanceRef (member %s 0)))\n"
				"\t\t\t\t\t))\n",
				mangled_name, mangled_name, mangled_name);
		/* connect carry outs with carry ins */
		for(idx=1;idx<width;idx++){
			fprintf(out,"\t\t\t\t(net NET_%s_%ld (joined\n",
					mangled_name, idx);
			fprintf(out,"\t\t\t\t\t(portRef CI (instanceRef (member %s %ld)))\n",
					mangled_name, idx);
			fprintf(out,"\t\t\t\t\t(portRef COUT (instanceRef (member %s %ld)))\n",
					mangled_name, idx-1);
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
			fprintf(out,"\t\t\t\t(instance (array (rename BUFZ_%s \"%s\") %d)\n",
				mangle_edif_name(ivl_lpm_name(net)),
				ivl_lpm_name(net), width);
			fprintf(out,"\t\t\t\t\t(viewRef %s (cellRef %s)))\n",
				current_library[i_b].viewref, current_library[i_b].cellref);
			for(idx=0;idx<width;idx++)
				fprintf(out,"\t\t\t\t(net NET_%s_%d (joined\n"
					    "\t\t\t\t\t(portRef S (instanceRef (member %s %d)))\n"
					    "\t\t\t\t\t(portRef Q (instanceRef (member BUFZ_%s %d)))\n"
					    "\t\t\t\t\t))\n", 
					mangle_edif_name(ivl_lpm_name(net)), idx, 
					mangle_edif_name(ivl_lpm_name(net)), idx, 
					mangle_edif_name(ivl_lpm_name(net)), idx);
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
	fprintf(out, "\t\t\t\t\t(port (rename %s  \"%s\") (property PIN_LOCATION (string \"%s\")) ", 
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


static void show_signal_as_instance(ivl_scope_t current_scope, ivl_signal_t net)
{
long pin,idx;
long i,i_p;
char *pad;
ivl_nexus_t nex;
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
                mangle_edif_name(pad),
                pad);
        fprintf(out,"\t\t\t\t\t(portRef %s)\n", mangle_edif_name(ivl_signal_attr(net, "PAD")));
        fprintf(out,"\t\t\t\t\t(portRef %s (instanceRef %s))\n",
                current_library[i_p].port_name[0],
                mangle_edif_name(pad));
        fprintf(out,"\t\t\t\t\t))\n"); /* ) of net and joined*/
        }
for(pin=0;pin<ivl_signal_pins(net);pin++){
        nex=ivl_signal_pin(net, pin);
        for (idx=0;idx<ivl_nexus_ptrs(nex);idx++){
                ivl_net_const_t con;
                ivl_net_logic_t log;
                ivl_lpm_t lpm;
                ivl_signal_t sig;
                ivl_nexus_ptr_t ptr=ivl_nexus_ptr(nex, idx);
                if((con=ivl_nexus_ptr_con(ptr))!=NULL){
                        const char*bits=ivl_const_bits(con);
                        unsigned pin=ivl_nexus_ptr_pin(ptr);
                        if(bits[pin]=='1'){
                                fprintf(out,"\t\t\t\t(instance ONE_%s_%d (viewRef ONE (cellRef ONE)))\n",
                                        mangle_edif_name(ivl_nexus_name(nex)),idx);
                                } else {
                                fprintf(out,"\t\t\t\t(instance ZERO_%s_%d (viewRef ZERO (cellRef ZERO)))\n",
                                        mangle_edif_name(ivl_nexus_name(nex)),idx);
                                }
                        }
		} 
	}
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

static void show_signal(ivl_scope_t current_scope, ivl_signal_t net)
{
long pin,idx;
long i,i_p;
char *pad;
ivl_nexus_t nex;
char s[2000];

if(ivl_signal_pins(net)<1)return;

for(pin=0;pin<ivl_signal_pins(net);pin++){
	nex=ivl_signal_pin(net, pin);
	sprintf(s, "%s %s", ivl_scope_name(current_scope), ivl_nexus_name(nex));
	idx=add_string(nexuses, s);
	if(nexuses->data[idx]!=NULL)continue;
	nexuses->data[idx]=nex;

	if(ivl_signal_pins(net)>1)
		fprintf(out, "\t\t\t\t(net (rename %s_%ld \"%s[%ld]\") (joined\n", 
      			mangle_edif_name(ivl_signal_basename(net)),pin,
			ivl_signal_basename(net),pin);
		else 
		fprintf(out, "\t\t\t\t(net (rename %s \"%s\") (joined\n", 
      			mangle_edif_name(ivl_signal_basename(net)),
			ivl_signal_basename(net));

	for (idx=0;idx<ivl_nexus_ptrs(nex);idx++){
		ivl_net_const_t con;
		ivl_net_logic_t log;
		ivl_lpm_t lpm;
		ivl_signal_t sig;
		ivl_nexus_ptr_t ptr=ivl_nexus_ptr(nex, idx);

		if((sig=ivl_nexus_ptr_sig(ptr))){
                        i=compare_scope_names(ivl_scope_name(current_scope),ivl_signal_basename(sig), ivl_signal_name(sig));
                        if(i>=0){

		  		if((i_p=find_pad_cell(sig))>=0)
       		              		fprintf(out, "\t\t\t\t\t(portRef %s (instanceRef %s))\n",
               		              		current_library[i_p].port_name[1],
               		              		mangle_edif_name(ivl_signal_attr(sig, "PAD")));

				if((ivl_signal_port(sig)!=IVL_SIP_NONE)){
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
		        if(!compare_scope_names(ivl_scope_name(current_scope), ivl_logic_basename(log), ivl_logic_name(log))){
				i=find_logic_cell(ivl_logic_type(log));
				if(i>=0)
					fprintf(out, "\t\t\t\t\t(portRef %s (instanceRef %s))\n", current_library[i].port_name[ivl_nexus_ptr_pin(ptr)], mangle_edif_name(ivl_logic_basename(log)));
				} else {
				fprintf(stderr,"Unrecognized logic cell %s (type %d)\n",
					 ivl_logic_name(log), ivl_logic_type(log));
				}	
		  	} else 
		if((lpm=ivl_nexus_ptr_lpm(ptr))!=NULL){
			show_lpm_as_portref(current_scope, nex, lpm);
			} else 
		if((con=ivl_nexus_ptr_con(ptr))!=NULL){
			const char*bits=ivl_const_bits(con);
			fprintf(out,"\t\t\t\t\t(portRef Q ");
			if(bits[ivl_nexus_ptr_pin(ptr)]=='1'){
				fprintf(out,"(instanceRef ONE_%s_%d))\n",
					mangle_edif_name(ivl_nexus_name(nex)),idx);
				} else {
				fprintf(out,"(instanceRef ZERO_%s_%d))\n",
					mangle_edif_name(ivl_nexus_name(nex)),idx);
				}
		 	} else {
			fprintf(stderr, " unrecognized nexus connection in nexus %s 0x%08x 0x%08x\n", ivl_nexus_name(nex), ((unsigned *)ptr)[0], ((unsigned *)ptr)[1]);
		  	}
/*
		fprintf(stderr,"Nexus connection (%s) %d 0x%08x 0x%08x\n", ivl_nexus_name(nex), idx,  ((unsigned *)ptr)[0], ((unsigned *)ptr)[1]);  		
*/
	    	}
	fprintf(out,"\t\t\t\t)\n"); /* ) for  joined */
	if(ivl_signal_attr(net, "PAD")!=NULL)
      		fprintf(out, "\t\t\t\t(property PAD_LOCATION (string \"%s\"))\n",
                        ivl_signal_attr(net, "PAD"));
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

static void show_logic(ivl_scope_t current_scope, ivl_net_logic_t net)
{
long npins, idx;
long library_cell;
char *name=(char *)ivl_logic_basename(net);
if(compare_scope_names(ivl_scope_name(current_scope), ivl_logic_basename(net), ivl_logic_name(net)))
      		return;

library_cell=find_logic_cell(ivl_logic_type(net));
if(library_cell<0){
	fprintf(stderr,"Standard cell %s is not present in the library, ignoring\n", ivl_logic_name(net));
	return;
	}
fprintf(out, "\t\t\t\t(instance (rename %s \"%s\")\n",mangle_edif_name(name), name);
fprintf(out, "\t\t\t\t\t(viewRef %s (cellRef %s))\n", current_library[library_cell].viewref, current_library[library_cell].cellref );
fprintf(out, "\t\t\t\t\t)\n"); /* ) of instance */

npins=ivl_logic_pins(net);
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

static int show_scope_as_instance(ivl_scope_t net, ivl_scope_t current_scope)
{
char *buf;
switch (ivl_scope_type(net)){
	case IVL_SCT_MODULE:
	        if(!compare_scope_names(ivl_scope_name(current_scope), ivl_scope_basename(net), ivl_scope_name(net))){
			fprintf(out, "\t\t\t\t(instance (rename %s \"%s\")\n", 
						mangle_edif_name(ivl_scope_name(net)),
						ivl_scope_name(net));
			buf=(char *)mangle_edif_name(ivl_scope_tname(net));
			fprintf(out, "\t\t\t\t\t(viewRef %s (cellRef %s))\n", buf, buf);
			fprintf(out, "\t\t\t\t\t)\n"); /* ) of instance */
			}
		break;
	default:
	}
return 0;
}

static int show_scope(ivl_scope_t net, void*x)
{
long idx;

switch(ivl_scope_type(net)){
	case IVL_SCT_MODULE:
	    	idx=add_string(cells, mangle_edif_name(ivl_scope_tname(net)));
	    	if(cells->data[idx]!=NULL)break;
	    	fprintf(stderr, "Processing module: %s (%u signals, %u logic)\n",
	    	  	ivl_scope_tname(net), ivl_scope_sigs(net),
	      	  	ivl_scope_logs(net));

	    	cells->data[idx]=net; /* mark this scope */
	    	/* define children first */
	    	ivl_scope_children(net, show_scope, 0);
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
		for(idx=0;idx<ivl_scope_sigs(net);idx++)
			show_signal_as_instance(net, ivl_scope_sig(net, idx));
	    	ivl_scope_children(net, show_scope_as_instance, net);
            	for (idx=0;idx<ivl_scope_logs(net);idx++)
            	    	show_logic(net, ivl_scope_log(net, idx));
            	for (idx=0;idx<ivl_scope_lpms(net);idx++)
	            	show_lpm(ivl_scope_lpm(net, idx));
	    	for (idx=0;idx<ivl_scope_sigs(net);idx++)
	    	    	show_signal(net, ivl_scope_sig(net, idx));
		    
	    	fprintf(out, "\t\t\t\t)\n"); /* ) of contents */
	    	fprintf(out, "\t\t\t))\n"); /* ) of cell and view*/
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
return 1;
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
nexuses=new_string_cache();
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
	    "\t\t\t\t)\n", "alpha edif output module"); /* replace with real version string later */
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

