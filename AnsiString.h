// See LICENSE file for usage restrictions.
//---------------------------------------------------------------------------
#ifndef AnsiStringH
#define AnsiStringH
//---------------------------------------------------------------------------
#include <string.h>
#include <windows.h>
#include <stdio.h>
using namespace std;
#ifdef _DEBUG
#define tcheck if(this==NULL){throw ("Argh! This is NULL!!!!");}
#else
#define tcheck
#endif
#pragma option push -w-inl -vi-
#include <stdlib.h>
#include <math.h>
#ifndef HPROCESSHEAP
#define HPROCESSHEAP
static HANDLE hProcessHeap=NULL;
#endif  //HPROCESSHEAP
#ifndef AutoInitStringHeap
#define AutoInitStringHeap
#define TAUTOINITSTRINGHEAP \
typedef class TAutoInitStringHeap \
{ \
public: \
        __inline __fastcall TAutoInitStringHeap(HANDLE *AA) \
        { \
                if(!*AA) \
                { \
                        *AA=::GetProcessHeap(); \
                } \
        } \
}TAutoInitStringHeapL;
TAUTOINITSTRINGHEAP
static TAutoInitStringHeapL AutoInitStringHeapU(&hProcessHeap);
#endif //AutoInitStringHeap
typedef class AnsiString
{
friend AnsiString __fastcall operator +(const char *lhs, const AnsiString& rhs)
{
        AnsiString Res=AnsiString(lhs)+rhs;
        return Res;
};
friend AnsiString __fastcall operator +(const wchar_t *lhs, const AnsiString& rhs)
{
        AnsiString Res=AnsiString(lhs)+rhs;
        return Res;
};
private:
        char *Data;
        void __fastcall Free(char *U)
        {
                if(U)
                {
                        //__try{::LocalFree(U);}__except(1){}
                        if(!HeapValidate(hProcessHeap,0,U))
                        {
                                return;
                        }
                        ::HeapFree(hProcessHeap,0,U);
                        //__try{::HeapFree(hProcessHeap,0,U);}__except(1){}
                        //__try{::VirtualFree(U,0,MEM_RELEASE);}__except(1){}
                }
        }
        void __fastcall Free(void)
        {
                tcheck
                if(Data!=NULL)
                {
                        try{Free(Data);}catch(...){}
                        Data=NULL;
                }
        }
        char * __fastcall Alloc(int l) const
        {
                if(!hProcessHeap)hProcessHeap=::GetProcessHeap();
                //return (char *)::LocalAlloc(LMEM_FIXED,l*sizeof(*Data));
                return (char *)::HeapAlloc(hProcessHeap,HEAP_GENERATE_EXCEPTIONS,l*sizeof(*Data));
                //return (char *)::VirtualAlloc(NULL,l*sizeof(*Data),MEM_COMMIT,PAGE_READWRITE);
        }
        void __fastcall Alloc(void)
        {
                if(Data!=NULL){Free();}
                Data=Alloc(1);
                Data[0]='\0';
        }
        __fastcall AnsiString(int *a)
        {
                tcheck
                Data=(char *)a;
        }
protected:
public:
        char * c_str(void) const
        {
                tcheck
                return Data;
        };
        int Length(void) const
        {
                tcheck
                return Data?(strlen(Data)):0;
        };
        __fastcall ~AnsiString(void) noexcept(false)
        {
                tcheck
                Free();
        }
        __fastcall AnsiString(const char *U)
        {
                tcheck
                if(U==NULL){Data=NULL;/*Alloc();*/return;}
                int l=strlen(U);
                if(l<1){Data=NULL;return;}
                Data = Alloc(l+1);
                strcpy(Data,U);
        }
        __fastcall AnsiString(const wchar_t *U)
        {
                tcheck
                BOOL dummy;
                if(U==NULL){Data=NULL;/*Alloc();*/return;}
                int l=wcslen(U);
                if(l<1){Data=NULL;return;}
                Data=Alloc(l+1);
                Data[l]=0;
                WideCharToMultiByte(
                        CP_ACP, 0, U, -1, Data, wcslen(U),NULL,&dummy
                );
        }
        __fastcall AnsiString(const char &U)
        {
                tcheck
                if(U=='\0'){Data=NULL;/*Alloc();*/return;}
                Data=Alloc(2);
                Data[1]='\0';
                Data[0]=U;
        }
        __fastcall AnsiString(void)
        {
                tcheck
                Data=NULL;
                //Alloc();
        }
        __fastcall AnsiString(const AnsiString &U)
        {
                tcheck
                if(U==""){Data=NULL;/*Alloc();*/return;}
                int l=strlen(U.Data);
                if(l<1){Data=NULL;return;}
                Data=Alloc(l+1);
                Data[l]=0;
                strcpy(Data,U.Data);
        }
        __fastcall AnsiString(const int U)
        {
                tcheck
                Data=Alloc(34);
                memset(Data,0,34);
                itoa(U,Data,10);
        }
        __fastcall AnsiString(const unsigned int U)
        {
                tcheck
                Data=Alloc(34);
                memset(Data,0,34);
                ultoa(U,Data,10);
        }
	__fastcall AnsiString(const __int64 U)
	{
                tcheck
		Data=Alloc(34);
		memset(Data,0,34);
		_i64toa(U,Data,10);
	}
	__fastcall AnsiString(const unsigned __int64 U)
	{
                tcheck
		Data=Alloc(34);
		memset(Data,0,34);
		_ui64toa(U,Data,10);
	}
        __fastcall AnsiString(const double U)
        {
                tcheck
                Data=Alloc(37);
                memset(Data,0,sizeof(char[37]));
                sprintf(Data,"%f",U);
        }
        AnsiString& __fastcall operator=(const AnsiString &U)
        {
                tcheck
                if(&U==this){return *this;}
                Free();
                if(U.Data==NULL){/*Alloc();*/return *this;}
                int l=strlen(U.Data);
                if(l<1)return *this;
                Data = Alloc(l+1);
                strcpy(Data,U.Data);
                return *this;
        }
        AnsiString& __fastcall operator=(const int &U)
        {
                tcheck
                Free();
                Data = Alloc(34);
                memset(Data,0,34);
                itoa(U,Data,10);
                return *this;
        }
        void __fastcall operator=(const char *U)
        {
                tcheck
                Free();
                if(U==NULL){/*Alloc();*/return;}
                int l=strlen(U);
                if(l<1)return;
                Data = Alloc(l+1);
                strcpy(Data,U);
        }
        bool __fastcall operator==(const AnsiString &rhs) const
        {
                tcheck
                if(rhs.Data==NULL)
                {
                        if(Data==NULL)return true;
                        if(strlen(Data)>0)return false; else return true;
                }
                if(Data==NULL)
                {
                        if(rhs.Data==NULL)return true;
                        if(strlen(rhs.Data)>0)return false; else return true;
                }
                return (strcmp(Data,rhs.Data)==0);
        }
        bool __fastcall operator==(const char *rhs) const
        {
                tcheck
                if(rhs==NULL)
                {
                        if(Data==NULL)return true;
                        if(strlen(Data)>0)return false; else return true;
                }
                if(Data==NULL)
                {
                        if(rhs==NULL)return true;
                        if(strlen(rhs)>0)return false; else return true;
                }
                return (strcmp(Data,rhs)==0);
        }
        bool __fastcall operator!=(const AnsiString &rhs) const
        {
                tcheck
                if(rhs.Data==NULL)
                {
                        if(Data==NULL)return false;
                        if(strlen(Data)>0)return true; else return false;
                }
                if(Data==NULL)
                {
                        if(rhs.Data==NULL)return false;
                        if(strlen(rhs.Data)>0)return true; else return false;
                }
                return (strcmp(Data,rhs.Data)!=0);
        }
        bool __fastcall operator!=(const char *rhs) const
        {
                tcheck
                if(rhs==NULL)
                {
                        if(Data==NULL)return false;
                        if(strlen(Data)>0)return true; else return false;
                }
                if(Data==NULL)
                {
                        if(rhs==NULL)return false;
                        if(strlen(rhs)>0)return true; else return false;
                }
                return (strcmp(Data,rhs)!=0);
        }
        AnsiString __fastcall operator+(const AnsiString& rhs) const
        {
                tcheck
                int l=(Data)?(strlen(Data)):0;
                int il=(rhs.Data)?(strlen(rhs.Data)):0;
                if(il<1)
                {
                        if(l<1)
                        {
                                return (int *)NULL;
                        }
                        char *nData=Alloc(l+1);
                        strcpy(nData,Data);
                        return (int *)nData;
                }
                if(l<1)
                {
                        char *nData=Alloc(il+1);
                        strcpy(nData,rhs.Data);
                        return (int *)nData;
                }
                char *nData=Alloc(l+il+1);
                strcpy(nData,Data);
                strcat(nData,rhs.Data);
                return (int *)nData;
        }
        void __fastcall operator+=(const AnsiString& rhs)
        {
                tcheck
                if(rhs.Data==NULL)return;
                int l=strlen(rhs.Data);
                if(l<1)return;
                int tl=(Data)?(strlen(Data)):0;
                if(tl<1)
                {
                        Free();
                        Data=Alloc(l+1);
                        strcpy(Data,rhs.Data);
                        return;
                }
                char *nData=Alloc(tl+l+1);
                strcpy(nData,Data);
                strcat(nData,rhs.Data);
                Free();
                Data=nData;
        }
        int __fastcall Pos(AnsiString subStr)
        {
                tcheck
                if(Data==NULL || Data[0]=='\0')return 0;
                if(subStr.Data==NULL || subStr.Data[0]=='\0')return 0;
                int tl=this->Length();
                int ol=subStr.Length();
                if(ol>tl)return 0;
                char *SData;
                for(int i=0;i<tl;i++)
                {
                        SData=Data+i;
                        if(strncmp(SData,subStr.Data,ol)==0)
                        {
                                return i+1;
                        }
                }
                return 0;
        }
        AnsiString __fastcall Trim(void) const
        {
			int L=(Data)?(strlen(Data)):0;
			if(L<1){return (int *)NULL;}
			int I=1;
			char *nData;
			while((I<=L) && (Data[I-1]<=' '))I++;
			if(I > L){nData=NULL;}else
			{
				while(Data[L-1]<=' ')L--;
							nData=Alloc(L-I+2);
							memcpy(nData,Data+I-1,sizeof(*Data)*(L-I+1));
							nData[L-I+1]=0;
			}
			return (int *)nData;
        };
        AnsiString __fastcall SubString(int index, int count) const
        {
                tcheck
                index=(index<1)?0:index-1;
                int l=(Data)?(strlen(Data)):0;
                if(l<1){return (int *)NULL;}
                count=(count+index<count)?l:count;
                count=(index+count>l)?(l-index):count;
                count=(count<0)?0:count;
                char *nData=Alloc(count+1);
                nData[count]='\0';
                if(count>0)
                {
                        for(l=0;l<count;l++)
                        {
                                nData[l]=Data[l+index];
                        }
                }
                return (int *)nData;
        }
        const AnsiString& __fastcall Delete(int index, int count)
        {
                tcheck
                index=(index<1)?0:index-1;
                int l=Data?(strlen(Data)):0;
                if(l<1)
                {
                        Free();
                        return *this;
                }
                count=(index+count>l)?(l-index):count;
                count=(count<0)?0:count;
                if(count>0)
                {
                        int nl=l-count;
                        char *nDest = Alloc(nl+1);
                        nDest[nl]='\0';
                        if(index>0)
                        {
                                char t=Data[index];Data[index]='\0';
                                strcpy(nDest,Data);
                                Data[index]=t;
                        }
                        if(index+count<l)
                        {
                                strcpy(nDest+index,Data+index+count);
                        }
                        Free();
                        Data=nDest;
                }
                return *this;
        }
	const AnsiString& __fastcall Insert(const AnsiString& str, int index)
	{
		tcheck
                index--;
                if(index<0){index=0;}
		int il=(str.Data)?(strlen(str.Data)):0;
		if(il<1)return *this;
                int l=(Data)?(strlen(Data)):0;
                if(l<1)
                {
                        Free();
                        Data=Alloc(il);
                        strcpy(Data,str.Data);
                        return *this;
                }
                if(index>=l){index=l;}
                char *OData=Data;
                Data=NULL;
                Data=Alloc(il+l+1);
                strcpy(Data,OData);
                char *r=Data;
                r+=index;
                strcpy(r,str.Data);
                r+=il;
                char *s=OData;
                s+=index;
                strcpy(r,s);
                Free(OData);
                return *this;
	}
        AnsiString __fastcall UpperCase(void) const
        {
                tcheck
                int l=Data?(strlen(Data)):0;
                if(l<1){return (int *)NULL;}
                char *nData=Alloc(l+1);
                nData[l]=0;
                if(Data!=NULL)
		{
			strcpy(nData,Data);
			strupr(nData);
		}
                /*for(int i=0;i<l;i++)
                {
                        nData[i]=toupper(nData[i]);
                }*/
                return (int *)nData;
        }
        AnsiString __fastcall LowerCase(void) const
        {
                tcheck
                int l=Data?(strlen(Data)):0;
                if(l<1){return (int *)NULL;}
                char *nData=Alloc(l+1);
                nData[l]=0;
                if(Data!=NULL)
		{
			strcpy(nData,Data);
			strlwr(nData);
		}
                /*for(int i=0;i<l;i++)
                {
                        nData[i]=tolower(nData[i]);
                }*/
                return (int *)nData;
        }
        AnsiString __fastcall Unique(void) const
        {
                return *this;
        }
        double __fastcall ToDouble(void)
        {
                double result;
                if(Data==NULL)throw AnsiString("Not a valid floating point value.");
                result = atof(Data);
                if(result==-HUGE_VAL || result==HUGE_VAL)
                {
                        throw AnsiString("Not a valid floating point value.");
                }
                return result;
        }
        int __fastcall ToIntDef(int def)
        {
                if(Data==NULL)return def;
                if(Data[0]==0)return def;
                char *ec;
                char **eec=&ec;
                int Res=strtol(Data,eec,0);
                if(Res==0)
                {
                        if(strlen(ec)!=0)
                        {
                                Res=def;
                        }
                }
                return Res;
        }
        AnsiString __fastcall SetLength(int Len)
        {
                int ol;
                if(Data!=NULL){ol=strlen(Data);}else{ol=0;}
                if(Len<=0)
                {
                        Free();
                        return *this;
                }
                char *nData=Alloc(Len+1);
                if(ol)
                {
                        if(ol>=Len)
                        {
                                memcpy(nData,Data,Len);
                        }
                        else
                        {
                                strcpy(nData,Data);
                        }
                        nData[Len]=0;
                        Free();
                        Data=nData;
                }
		else
		{
			Free();
			Data=nData;
			memset(Data,32,Len);
			Data[Len]=0;
		}
                return *this;
        }
	bool __fastcall IsEmpty(void) const
	{
		if(Data==NULL)return true;
		if(Data[0]=='\0')return true;
		return false;
	}
        char& __fastcall operator [](const int idx) const { return Data[idx-1]; }
}*PAnsiString;
/*extern AnsiString __fastcall operator +(const char*, const AnsiString&)
{
        return "";
};*/
#pragma option pop
#ifdef _DEBUG
#undef tcheck
#endif
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------

