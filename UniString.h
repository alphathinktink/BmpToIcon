// See LICENSE file for usage restrictions.
//---------------------------------------------------------------------------
#ifndef UniStringH
#define UniStringH
//---------------------------------------------------------------------------
#include <string.h>
#include <stddef.h>
#include <windows.h>
#include <stdio.h>
using namespace std;
#pragma option push -w-inl -vi-
#include <stdlib.h>
#include <math.h>
#pragma defineonoption INCLUDE_VCL -tWV
#ifndef INCLUDE_VCL
#ifndef VCL_H
#ifndef __VCL0_H__
#define NO_INCLUDE_VCL
#endif
#endif
#endif
#ifdef NO_INCLUDE_VCL
#include "AnsiString.h"
#else
#include <windows.h>
#include <vcl.h>
#endif
#ifdef _DEBUG
#define tcheck if(this==NULL || (ULONG_PTR)(this)<=0x0000FFFF){throw ("Argh! This is NULL!!!!");}
#else
#define tcheck
#endif
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
typedef class UniString
{
friend UniString __fastcall operator +(const wchar_t *lhs, const UniString& rhs)
{
        UniString Res=UniString(lhs)+rhs;
        return Res;
};
friend UniString __fastcall operator +(const char *lhs, const UniString& rhs)
{
        UniString Res=UniString(lhs)+rhs;
        return Res;
};
public:
        #ifdef _DEBUG
        AnsiString __fastcall GetDebugString(void)
        {
                tcheck
                return AnsiString(Data);
        };
        #endif
private:
        wchar_t *Data;
        void __fastcall Free(wchar_t *U)
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
        wchar_t * __fastcall Alloc(int l) const
        {
                if(!hProcessHeap)hProcessHeap=::GetProcessHeap();
                //return (wchar_t *)::LocalAlloc(LMEM_FIXED,l*sizeof(*Data));
                return (wchar_t *)::HeapAlloc(hProcessHeap,HEAP_GENERATE_EXCEPTIONS,l*sizeof(*Data));
                //return (wchar_t *)::VirtualAlloc(NULL,l*sizeof(*Data),MEM_COMMIT,PAGE_READWRITE);
        }
        void __fastcall Alloc(void)
        {
                if(Data!=NULL){Free();}
                Data=Alloc(1);
                Data[0]=L'\0';
        }
        __fastcall UniString(int *a)
        {
                tcheck
                Data=(wchar_t *)a;
        }
protected:
public:
        wchar_t * c_bstr(void) const
        {
                tcheck
                return Data;
        };
        wchar_t * c_str(void) const
        {
                tcheck
                return Data;
        };
        int Length(void) const
        {
                tcheck
                return Data?(wcslen(Data)):0;
        }
        __fastcall ~UniString(void) noexcept(false)
        {
                tcheck
                Free();
        }
        __fastcall UniString(const UniString *U)
        {
                *this=*U;
        }
        __fastcall UniString(const wchar_t *U)
        {
                tcheck
                if(U==NULL){Data=NULL;/*Alloc();*/return;}
                int i=wcslen(U);
                if(i<1){Data=NULL;return;}
                Data = Alloc(i+1);
                wcscpy(Data,U);
        }
        __fastcall UniString(const char *U)
        {
                tcheck
                if(U==NULL){Data=NULL;/*Alloc();*/return;}
                int i=strlen(U);
                if(i<1){Data=NULL;return;}
                Data=Alloc(i+1);
                Data[strlen(U)]=0;
                MultiByteToWideChar(
                        CP_ACP, 0, U, -1, Data, strlen(U)
                );
        }
        __fastcall UniString(const wchar_t &U)
        {
                tcheck
                if(U==L'\0'){Data=NULL;/*Alloc();*/return;}
                Data=Alloc(2);
                Data[1]=L'\0';
                Data[0]=U;
        }
        __fastcall UniString(void)
        {
                tcheck
                Data=NULL;
                //Alloc();
        }
        __fastcall UniString(const UniString &U)
        {
                tcheck
                if(U.Data==NULL){Data=NULL;/*Alloc();*/return;}
                int l=wcslen(U.Data);
                if(l<1){Data=NULL;return;}
                Data=Alloc(l+1);
                Data[l]=L'\0';
                wcscpy(Data,U.Data);
        }
        __fastcall UniString(const AnsiString &U)
        {
                tcheck
                if(U==""){Data=NULL;/*Alloc();*/return;}
                int l=strlen(U.c_str());
                if(l<1){Data=NULL;return;}
                Data=Alloc(l+1);
                Data[strlen(U.c_str())]=0;
                MultiByteToWideChar(
                        CP_ACP, 0, U.c_str(), -1, Data, strlen(U.c_str())
                );
        }
        __fastcall UniString(const int U)
        {
                tcheck
                Data=Alloc(34);
                memset(Data,0,sizeof(wchar_t[34]));
                _itow(U,Data,10);
        }
        __fastcall UniString(const unsigned int U)
        {
                tcheck
                Data=Alloc(34);
                memset(Data,0,sizeof(wchar_t[34]));
                _ultow(U,Data,10);
        }
	__fastcall UniString(const __int64 U)
	{
                tcheck
		Data=Alloc(34);
		memset(Data,0,sizeof(wchar_t[34]));
		_i64tow(U,Data,10);
	}
	__fastcall UniString(const unsigned __int64 U)
	{
                tcheck
		Data=Alloc(34);
		memset(Data,0,sizeof(wchar_t[34]));
		_ui64tow(U,Data,10);
	}
        __fastcall UniString(const double U)
        {
                tcheck
                Data=Alloc(37);
                memset(Data,0,sizeof(wchar_t[37]));
                swprintf(Data,L"%f",U);
        }
        const UniString& __fastcall operator=(const UniString &U)
        {
                tcheck
                if(&U==this)
                {
                        return *this;
                }
                Free();
                if(U.Data==NULL)
                {
                        //Alloc();
                        return *this;
                }
                int l=wcslen(U.Data);
                if(l<1)return *this;
                Data = Alloc(l+1);
                wcscpy(Data,U.Data);
                return *this;
        }
        const UniString& __fastcall operator=(const int &U)
        {
                tcheck
                Free();
                Data = Alloc(34);
                memset(Data,0,sizeof(wchar_t[34]));
                _itow(U,Data,10);
                return *this;
        }/**/
        void __fastcall operator=(const wchar_t *U)
        {
                tcheck
                Free();
                if(U==NULL)
                {
                        //Alloc();
                        return;
                }
                int l=wcslen(U);
                if(l<1)return;
                Data = Alloc(l+1);
                wcscpy(Data,U);
        }
        bool __fastcall operator==(const UniString &rhs) const
        {
                tcheck
                if(rhs.Data==NULL)
                {
                        if(Data==NULL)return true;
                        if(wcslen(Data)>0)return false; else return true;
                }
                if(Data==NULL)
                {
                        if(rhs.Data==NULL)return true;
                        if(wcslen(rhs.Data)>0)return false; else return true;
                }
                return (wcscmp(Data,rhs.Data)==0);
        }
        bool __fastcall operator==(const wchar_t *rhs) const
        {
                tcheck
                if(rhs==NULL)
                {
                        if(Data==NULL)return true;
                        if(wcslen(Data)>0)return false; else return true;
                }
                if(Data==NULL)
                {
                        if(rhs==NULL)return true;
                        if(wcslen(rhs)>0)return false; else return true;
                }
                return (wcscmp(Data,rhs)==0);
        }
        bool __fastcall operator!=(const UniString &rhs) const
        {
                tcheck
                if(rhs.Data==NULL)
                {
                        if(Data==NULL)return false;
                        if(wcslen(Data)>0)return true; else return false;
                }
                if(Data==NULL)
                {
                        if(rhs.Data==NULL)return false;
                        if(wcslen(rhs.Data)>0)return true; else return false;
                }
                return (wcscmp(Data,rhs.Data)!=0);
        }
        bool __fastcall operator!=(const wchar_t *rhs) const
        {
                tcheck
                if(rhs==NULL)
                {
                        if(Data==NULL)return false;
                        if(wcslen(Data)>0)return true; else return false;
                }
                if(Data==NULL)
                {
                        if(rhs==NULL)return false;
                        if(wcslen(rhs)>0)return true; else return false;
                }
                return (wcscmp(Data,rhs)!=0);
        }
        UniString __fastcall operator+(const UniString& rhs) const
        {
                tcheck
                int l=(Data)?(wcslen(Data)):0;
                int il=(rhs.Data)?(wcslen(rhs.Data)):0;
                if(il<1)
                {
                        if(l<1)
                        {
                                return (int *)NULL;
                        }
                        wchar_t *nData=Alloc(l+1);
                        wcscpy(nData,Data);
                        return (int *)nData;
                }
                if(l<1)
                {
                        wchar_t *nData=Alloc(il+1);
                        wcscpy(nData,rhs.Data);
                        return (int *)nData;
                }
                wchar_t *nData=Alloc(wcslen(Data)+wcslen(rhs.Data)+1);
                wcscpy(nData,Data);
                wcscat(nData,rhs.Data);
                return (int *)nData;
        }
        void __fastcall operator+=(const UniString& rhs)
        {
                tcheck
                if(rhs.Data==NULL)return;
                int l=wcslen(rhs.Data);
                if(l<1)return;
                int tl=(Data)?(wcslen(Data)):0;
                if(tl<1)
                {
                        Free();
                        Data=Alloc(l+1);
                        wcscpy(Data,rhs.Data);
                        return;
                }
                wchar_t *nData=Alloc(tl+l+1);
                wcscpy(nData,Data);
                wcscat(nData,rhs.Data);
                Free();
                Data=nData;
        }
        int __fastcall Pos(UniString subStr)
        {
                tcheck
                if(Data==NULL || Data[0]==L'\0')return 0;
                if(subStr.Data==NULL || subStr.Data[0]==L'\0')return 0;
                int tl=this->Length();
                int ol=subStr.Length();
                if(ol>tl)return 0;
                wchar_t *SData;
                for(int i=0;i<tl;i++)
                {
                        SData=Data+i;
                        if(wcsncmp(SData,subStr.c_bstr(),ol)==0)
                        {
                                return i+1;
                        }
                }
                return 0;
        }
        UniString __fastcall Trim(void) const
        {
			int L=(Data)?(wcslen(Data)):0;
			if(L<1){return (int *)NULL;}
			int I=1;
			wchar_t *nData;
			while((I<=L) && (Data[I-1]<=L' '))I++;
			if(I > L){nData=NULL;}else
			{
				while(Data[L-1]<=L' ')L--;
							nData=Alloc(L-I+2);
							memcpy(nData,Data+I-1,sizeof(*Data)*(L-I+1));
							nData[L-I+1]=0;
			}
			return (int *)nData;
        };
        UniString __fastcall SubString(int index, int count) const
        {
                tcheck
                index=(index<1)?0:index-1;
                int l=Data?(wcslen(Data)):0;
                if(l<1){return (int *)NULL;}
                count=(count+index<count)?l:count;
                count=(index+count>l)?(l-index):count;
                count=(count<0)?0:count;
                wchar_t *nData=Alloc(count+1);
                nData[count]=L'\0';
                if(count>0)
                {
                        for(l=0;l<count;l++)
                        {
                                nData[l]=Data[l+index];
                        }
                }
                return (int *)nData;
        }
        const UniString& __fastcall Delete(int index, int count)
        {
                tcheck
                index=(index<1)?0:index-1;
                int l=Data?(wcslen(Data)):0;
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
                        wchar_t *nDest = Alloc(nl+1);
                        nDest[nl]=L'\0';
                        if(index>0)
                        {
                                wchar_t t=Data[index];Data[index]=L'\0';
                                wcscpy(nDest,Data);
                                Data[index]=t;
                        }
                        if(index+count<l)
                        {
                                wcscpy(nDest+index,Data+index+count);
                        }
                        Free();
                        Data=nDest;
                }
                return *this;
        }
	const UniString& __fastcall Insert(const UniString& str, int index)
	{
		tcheck
                index--;
                if(index<0){index=0;}
		int il=str.Data?(wcslen(str.Data)):0;
		if(il<1)return *this;
                int l=Data?(wcslen(Data)):0;
                if(l<1)
                {
                        Free();
                        Data=Alloc(il);
                        wcscpy(Data,str.Data);
                        return *this;
                }
                if(index>=l){index=l;}
                wchar_t *OData=Data;
                Data=NULL;
                Data=Alloc(il+l+1);
                wcscpy(Data,OData);
                wchar_t *r=Data;
                r+=index;
                wcscpy(r,str.Data);
                r+=il;
                wchar_t *s=OData;
                s+=index;
                wcscpy(r,s);
                Free(OData);
                return *this;
	}
        UniString __fastcall UpperCase(void) const
        {
                tcheck
                int l=Data?(wcslen(Data)):0;
                if(l<1){return (int *)NULL;}
                wchar_t *nData=Alloc(l+1);
                nData[l]=0;
                if(Data!=NULL)
		{
			wcscpy(nData,Data);
			_wcsupr(nData);
		}
                /*for(int i=0;i<l;i++)
                {
                        nData[i]=towupper(nData[i]);
                }*/
                return (int *)nData;
        }
        UniString __fastcall LowerCase(void) const
        {
                tcheck
                int l=Data?(wcslen(Data)):0;
                if(l<1){return (int *)NULL;}
                wchar_t *nData=Alloc(l+1);
                nData[l]=0;
                if(Data!=NULL)
		{
			wcscpy(nData,Data);
			_wcslwr(nData);
		}
                /*for(int i=0;i<l;i++)
                {
                        nData[i]=towlower(nData[i]);
                }*/
                return (int *)nData;
        }
        UniString __fastcall Unique(void) const
        {
                return *this;
        }
        double __fastcall ToDouble(void)
        {
                double result;
                if(Data==NULL)throw AnsiString("Not a valid floating point value.");
                result = _wtof(Data);
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
                wchar_t *ec;
                wchar_t **eec=&ec;
                int Res=wcstol(Data,eec,0);
                if(Res==0)
                {
                        if(wcslen(ec)!=0)
                        {
                                Res=def;
                        }
                }
                return Res;
        }
        UniString __fastcall SetLength(int Len)
        {
                int ol;
                if(Data!=NULL){ol=wcslen(Data);}else{ol=0;}
                if(Len<=0)
                {
                        Free();
                        return *this;
                }
                wchar_t *nData=Alloc(Len+1);
                if(ol)
                {
                        if(ol>=Len)
                        {
                                memcpy(nData,Data,Len*2);
                        }
                        else
                        {
                                wcscpy(nData,Data);
                        }
                        nData[Len]=0;
                        Free();
                        Data=nData;

                }
                else
                {
                        Free();
                        Data=nData;
                        memset(Data,32,Len*2);
                        Data[Len]=0;
                }
                return *this;
        }
	bool __fastcall IsEmpty(void)
	{
		if(Data==NULL)return true;
		if(Data[0]==L'\0')return true;
		return false;
	}
        wchar_t& __fastcall operator [](const int idx) const { return Data[idx-1]; }
}*PUniString;
/*extern UniString __fastcall operator +(const char*, const UniString&)
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

