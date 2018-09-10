#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <math.h>

int r[16],res,signextimm,ra,numofr,numofj,numofi,numofmemacc,numoftakenbr;
unsigned int pc=0;
unsigned int memory[0x100000];
unsigned int instruction,opcode,offset;
unsigned int opcode;
unsigned int rn,rd,rm,shamt,function,immediate,control,address,writedata1,writedata2,writedest1,writedest2,memread;
bool regdst,jump,branch,memtoreg,memwrite,alusrc,regwrite,print;
unsigned int cond,class;
bool memop,  p, u, l,b,s,w;
bool z=1;
bool c=1;
bool n=1;
bool v=0;
unsigned int read1, read2, read3, read4, read5, read6;
unsigned int aluinput1, aluinput2, aluinput3,cpsr, aluresult;
unsigned int cycle=0;

void fetch();
void decode();
void execute();
void memacc();
void wrbck();
int rotimm(int rotate, int imm);
int numofsetbits(int);
int nosb=0;

int main(void)
{
	FILE *stream;
	unsigned int line;
	size_t read = 0;
	stream = fopen("simple4.arm.bin","rb");
	print=1;
        if (stream == NULL)
		exit(EXIT_FAILURE);
//	setting();
	memset(memory,0,sizeof(memory));
	while(1)
	{
		read=fread(&line,4,1,stream);
//		line=(unsigned int)ntohl((uint32_t)line);
		memory[pc]=line;
		if(read==0)break;
		pc++;
	}
	memset(r,0,sizeof(r));
	r[13]=0x400000;
	r[11]=0;
	r[14]=-3;
	r[15]=0;
	if(print)if(print)printf("============================new=============================\n");
	while(r[15]>-1)
	{
		fetch();
		decode();
		execute();
		memacc();
		wrbck();
		for(int i=0; i<16;i++)
		{
			if(print)printf("r%d : %x\n",i,r[i]);
		}
	}
	printf("return value : %d\n",r[0]);
	printf("All done\n");
	return 0;
}

void fetch()
{
	++cycle;
	if(print)if(print)printf("================cycle:%d===strat=pc:%x======================\n",cycle,r[15]);
	instruction=memory[(r[15])/4];
	r[15]=r[15]+4;
	if(print)if(print)printf("\n%8.8x\n",instruction);
}

void decode()
{
	int tread2,tread3;
	cond=(0xf0000000&instruction)>>28;
	if(cond==0b1101)
	{
		if(print)printf("%d%d%d%d\n",z,c,n,v);
		if(z==1||n!=v)
		{
		}
		else
		{
			fetch();
		}
	}

	if(cond==0b0001)
	{
		if(z==0)
		{
		}
		else
		{
			fetch();
		}
	}

	if(cond==0b1100)
	{
		if(z==0&&n==v)
		{
		}
		else
		{
			fetch();
		}
	}

	class=(0x0e000000&instruction)>>25;	
	if(print)printf("%x\n",cond);
	if((0x1<class)&&(class<0x5))
	{
		if(print)printf("memory access enable\n");
		memop=1;
		p=(instruction&0x01000000)>>24;
		u=(instruction&0x00800000)>>23;
		b=(instruction&0x00400000)>>22;
		w=(instruction&0x00200000)>>21;
		l=(instruction&0x00100000)>>20;
	}
	else
	{
		if(print)printf("memory access disable\n");
		p=0;
		u=0;
		b=0;
		w=0;
		l=0;
		memop=0;
	}
	
	if(cond==0b1111)//uncondictional instruction
	{
	}
	
	else
	{
		read1=(instruction&0x01e00000)>>21;
		read2=(instruction&0x000f0000)>>16;
		tread2=read2;
		read2=r[read2];
		read3=(instruction&0x0000f000)>>12;
		tread3=read3;
		read3=r[read3];
		read4=(instruction&0x00000f00)>>8;
		read4=r[read4];
		if(!memop&&class!=5)
		{
			s=(instruction&0x00100000)>>20;
		}
		else
		{
			s=0;
		}
		writedest2=tread3;
		writedest1=tread2;
		switch(class)
		{
			case 0x0 ://need to be shift
				if(((instruction&0x00000010)>>4)==0)
				{
					if(print)printf("read4 modified\n");
					read4=(instruction&0x00000f80)>>7;
				}

				read5=(instruction&0x00000060)>>5;
				read6=(instruction&0x0000000f);
				read6=r[read6];
				if(print)printf("data processing shift\n");
				if(print)printf("%x %x %x %x %x %x\n",read1,read2,read3,read4,read5,read6);
				
				break;
			case 0x1 ://all need rotate shift
				if(print)printf("datadata processing immediate\n");
				read4=(instruction&0x00000f00)>>8;
				read5=(instruction&0x000000ff);
				opcode=(0x01e00000&instruction)>>21;
				
				break;
			case 0x2 ://enable memory access
				if(print)printf("load/stroe immediate offset\n");
				read4=(instruction&0xfff);
				break;
			case 0x3 ://memory access
				if(print)printf("load/store register offset\n");
				read4=(instruction&0x00000f80)>>7;
				read5=(instruction&0x00000060)>>5;
				read6=(instruction&0x0000000f);
				read6=r[read6];
				break;
			case 0x4 ://memory access
				if(print)printf("load/store multiple\n");
				read3=(instruction&0x0000ffff);
			//	writedest1=read2;
				break;
			case 0x5 ://branch
				if(print)printf("branch and branch with link\n");
				read2=(instruction&0x00ffffff);
				if((read2&0x00800000)>>23)
				{
					read2=read2|0xff000000;
				}
				writedest2=15;
				if(((read1&0x8)>>3)==1)
				{
					if(print)printf("bl check \n");
					writedest1=14;
				}
				break;
			case 0x6 :
				if(print)printf("coprocessor load/store and double register transfers\n");
				break;
			case 0x7 :
				if(print)printf("coprocessor\n");
				break;
			default  :
				if(print)printf("undefined instruction\n");
				break;
		}
	}
}


void execute()
{

	//shift first
	if(class==0x0||class==0x3)
	{
		//original shift
		if(read1==0x9)
		{
			aluinput1=read6;
		}
		else if(read5==0x0)//logical shift left
		{
			aluinput1=read2;
			if(read4!=0)
			{
				aluinput2=read6<<read4;
			}
			else if(read4==0)
			{
				aluinput2=read6;
			}
		}
	}
	else if(class==0x1)
	{
		//rotate shift
		aluinput1=read2;
		aluinput2=rotimm(read4,read5);
		if(print)printf("%d %d\n",aluinput1, aluinput2);
		if(read1==0x8)
		{
			int temp1=(instruction&0x000f0000);
			int temp2=(instruction&0x00000fff);
			aluinput1=(temp1>>4)+temp2;
		}
		if(read1==0xa&&s==0)
		{
			if(print)printf("check");
			int temp1=(instruction&0x000f0000);
			int temp2=(instruction&0x00000fff);
			aluinput1=(temp1>>4)+temp2;
			aluinput1=aluinput1<<16;
		}
	}
	else if(class==0x5)
	{
		if(print)printf("%x\n",read2);
		aluinput1=read2<<2;
		if(print)printf("%x\n");
		aluinput2=r[15];
	}
	else if(class==0x2)
	{
		//do not shift
		//make link between read data and alu input
		aluinput2=read4;
		aluinput1=read2;
		aluinput3=read3;
	}
	else//0x04
	{
		aluinput1=read2;
		aluinput2=read3;
	}

	if(class==0x0)
	{
		if(!s)
		{
			if(read1==0b1001)
			{
				aluinput1=read6;
			}
		}
	}
	//alu operation
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////

	if(class==0x0||class==0x1)
	{
		//operation according to opcode
		//using switch
		switch(read1)
		{
			case 0x2://sub
				aluresult=aluinput1-aluinput2;
				break;
			case 0x3://rsb
				aluresult=aluinput2-aluinput1;
				break;
			case 0x4://add
				if(print)printf("aluinput1 : %d \t aluinput2 : %d\n",aluinput1,aluinput2);
				aluresult=aluinput1+aluinput2;
				break;
			case 0xd://mov
				aluresult=aluinput2;
				break;
			case 0x8://movw
				if(print)printf("check\n");
				aluresult=aluinput1;
				break;
/*			case 0xa://movt
				aluresult=aluinput1;
				break;*/
			case 0x9://bx
				aluresult=aluinput1&0xfffffffe;
				break;
			case 0xa://cmp
				if(s)
				{
					if(aluinput1<aluinput2)
					{	
						c=1;
						n=1;
					}
					else 
					{
						c=0;
						n=0;
					}
				
					aluresult=aluinput1-aluinput2;
					if(print)printf("%d\t%d\n",aluinput1,aluinput2);
				}
				else
				{
					aluresult=aluinput1;
				}
				if(s)
				{
					if(aluresult>0x7fffffff||aluresult<0x8000000)
						v=0;
					else v=1;
				}
				break;
			default :
				exit(EXIT_FAILURE);
				break;
		}
	}
	else if(class==0x4)
	{
		// rn just delivered to memacc as address
		nosb=numofsetbits(aluinput2);
		if(p)
		{
			if(u)
			{
				aluresult=aluinput1+4;
			}
			else
			{
				if(print)printf("%x\n",aluinput2);
				aluresult=aluinput1-(nosb*4);
				if(print)printf("%x\n",aluresult);
				if(print)printf("%d\n",nosb);
			}
		}
		else
		{
			if(u)
			{
				aluresult=aluinput1;
			}
			else
			{
				aluresult=aluinput1-(nosb*4)+4;
			}
		}
	}
	else if(class==0x02||class==0x03)
	{
		//operation for memory access addressing
		//preindexed and post indexed, both need to be delivered to memacc
		if(u)
		{
			aluresult=aluinput1+aluinput2;
		}
		else
		{
			if(print)printf("%x %x\n",aluinput1,aluinput2);
			aluresult=aluinput1-aluinput2;
		}

		if(b)
		{//should be in wrbck?
		}
		else
		{
		}
	}
	else if(class==0x5)
	{
		//input + current pc + 8
		if(print)printf("%x %x\n",aluinput1,aluinput2);
		aluresult=aluinput1+aluinput2+4;
	}
	if(print)printf("aluresult :0x%x\n",aluresult);
	if(s)
	{
		if(print)printf("cpsr updated\n");
		if(print)printf("%x\n",aluresult);
		if(aluresult>>31==1)
		{
			n=1;
		}
		else
		{
			n=0;
		}

		if(aluresult==0)
		{
			z=1;
		}
		else
		{
			z=0;
		}

		/*if(aluinput1+aluinput2!=(signed)aluresult||aluinput1-aluinput2==(signed)aluresult)
		{
			c=0;
		}
		else if(aluinput1+aluinput2==(signed)aluresult||aluinput1-aluinput2!=(signed)aluresult)
		{
			c=1;
		}

		if((aluinput1>0&&aluinput2>0&&aluresult<0)||(aluinput1<0&&aluinput2<0&&aluresult>0))
		{
			v=1;
		}
		else
		{
			v=0;
		}*/
		cpsr=(n<<3)|(z<<2)|(c<<1)|v;
		if(print)printf("cpsr : %d%d%d%d\n",n,z,c,v);
	}

}

void memacc()
{
	if(memop&&(class!=0x04))
	{
		if(p)
		{
			address=aluresult;
		}
		else
		{
			address=aluinput1;
		}
		if(l)
		{
			memread=memory[address/4];
			if(print)printf("loaded value : 0x%x\n",memread);
		}
		else
		{
			memory[address/4]=aluinput3;
			if(print)printf("stored value : 0x%x\n",memory[address/4]);
		}
		if(print)printf("address : 0x%x\n",address);
	}
	if(class==0x04)
	{
		address=aluresult;
		if(l)
		{
			for(int i=0;i<16;i++)
			{
				if((aluinput2&0b1)==1)
				{
					r[i]=memory[address/4];
					if(print)printf("R[%d]:%x:M[%x]\n",i,memory[address/4],address);
					address=address+4;
				}
				aluinput2=aluinput2>>1;
				
			}
		}
		else
		{
			for(int i=0;i<16;i++)
			{
				if((aluinput2&0b1)==1)
				{
					memory[address/4]=r[i];
					if(print)printf("M[%x]:%x:R[%d]\n",address,memory[address/4],i);
					address=address+4;
				}
				aluinput2=aluinput2>>1;
			}
		}
	}
}

void wrbck()
{
	if(s)
	{
		return;
	}
	if(memop&&(class!=0x04)&&(class!=0x5))
	{	
		if(p)
		{
			writedata1=aluresult;
		}
		else
		{
			writedata1=aluinput1;
		}

		if(l)
		{
			writedata2=memread;
			r[writedest2]=writedata2;
		}
		else
		{
		}
		
		if(w)
		{
			r[writedest1]=writedata1;
		}
	}
	else if(class==0x04)
	{
		if(w)
		{
			if(u)
			{
				r[writedest1]=aluinput1+4*nosb;
			}
			else
			{
				if(print)printf("check\n");
				r[writedest1]=aluinput1-4*nosb;
				if(print)printf("nosb : %d\n",nosb);
			}
		}
		else
		{

		}
	}
	else if(class==0x5)
	{
		writedata2=aluresult;
		r[writedest2]=writedata2;
		if(((read1&0x8)>>3)==1)
		{
			if(print)printf("bl check \n");
			writedata1=aluinput2;
			r[writedest1]=writedata1;
			if(print)printf("%d ,%x\n",writedest1,writedata1);
		}
	}
	else
	{
		if(read1==0xa)
		{
			writedata2=aluresult;
			r[writedest2]=r[writedest2]|writedata2;
		}
		else
		{
			writedata2=aluresult;
			r[writedest2]=writedata2;
		}
	}
}


int rotimm(int rot,int imm)
{
	if(rot>=4)
	{
		imm=imm<<(32-rot*2);
		return imm;
	}
	else if(rot==0)
	{
		return imm;
	}
	else
	{
		int temp;
		int temp2=1;
		for(int i=0;i<rot;i++)
		{
			temp2*=4;
		}
		temp=temp2;
		temp=temp-1;
		temp=imm&temp;
		imm=imm>>(2*rot);
		imm=imm|(temp<<(32-rot*2));
		return imm;
	}
}

int numofsetbits(int registerlist)
{
	int numofbits=0;
	for(int i; i<16; i++)
	{
		if((registerlist&1)==1)
		{
			numofbits++;
		}
		registerlist=registerlist>>1;
	}
	return numofbits;
}
