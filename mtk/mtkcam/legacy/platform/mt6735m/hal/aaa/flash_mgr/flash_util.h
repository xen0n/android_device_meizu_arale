#pragma once

class FlashUtil
{
    enum
    {
        FL_UTL_ERR_FILE_NOT_EXIST=-2000,
    };

public:
    static FlashUtil* getInstance();
    FlashUtil();
    virtual ~FlashUtil();
    //int convertPline(int type, void* out, void* in);
    //int cleanPlineConvertInterm();


    static void aaSub(void* arr, int* sub);

    static int getPropInt(const char* sId, int DefVal);
    static int getFileCount(const char* fname, int* fcnt, int defaultValue);
    static int setFileCount(const char* fname, int cnt);
    static int getMs();
    static void createDir(const char* dir);

    static int aaToBmp(void* arr, const char* aeF, const char* awbF);

    template <class T>
    static void flash_sortxy_xinc(int n, T* x, T* y)
    {
        int i;
        int j;
        for(i=0;i<n;i++)
        for(j=i+1;j<n;j++)
        {
            if(x[i]>x[j])
            {
                T tmp;
                tmp =x[i];
                x[i]=x[j];
                x[j]=tmp;
                tmp =y[i];
                y[i]=y[j];
                y[j]=tmp;
            }
        }
    }




    template <class T>
    static T flash_interp(T x1, T y1, T x2, T y2, T x)
    {
        return y1+ (y2-y1)*(x-x1)/(x2-x1);
    }

    template <class T>
    static T flash_calYFromXYTab(int n, T* xNode, T* yNode, T x)
    {
        T y=yNode[0];
        int i;
        T xst;
        T yst;
        T xed;
        T yed;
        xst=xNode[0];
        yst=yNode[0];
        if(x<xNode[0])
            x=xNode[0];
        else if(x>xNode[n-1])
            x=xNode[n-1];

        for(i=1;i<n;i++)
        {
            xed=xNode[i];
            yed=yNode[i];
            if(x<=xNode[i])
            {
                y=flash_interp(xst, yst, xed, yed, x);
                break;
            }
            xst=xed;
            yst=yed;
        }
        if(x<=xNode[0])
            y=yNode[0];
        else if(x>=xNode[n-1])
            y=yNode[n-1];
        return y;
    }
};


template <class T>
int arrayToBmp(const char* fname, T* r, T* g, T* b, int w, int h, double maxVal=255)
{
    char header[54];
    int* pN;
    short* pShort;
    header[0]='B';
    header[1]='M';
    pN = (int*)(header+2);
    *pN = 3*w*h+54;
    pN = (int*)(header+6);
    *pN = 0;
    pN = (int*)(header+0xa);
    *pN = 54;
    pN = (int*)(header+0xe);
    *pN = 0x28;
    pN = (int*)(header+0x12);
    *pN = w;
    pN = (int*)(header+0x16);
    *pN = h;
    pShort= (short*)(header+0x1A);
    *pShort=1;
    pShort= (short*)(header+0x1C);
    *pShort=24;
    pN = (int*)(header+0x1e);
    *pN = 0;
    pN = (int*)(header+0x22);
    *pN = 0;
    pN = (int*)(header+0x26);
    *pN = 2834;
    pN = (int*)(header+0x2a);
    *pN = 2834;
    pN = (int*)(header+0x2e);
    *pN = 0;
    pN = (int*)(header+0x32);
    *pN = 0;

    int lineBytes;
    lineBytes = ((w*3+3)/4)*4;
    unsigned char* data;
    unsigned char* data2;
    data = new unsigned char[lineBytes*h];
    int i;
    int j;
    int pos;
    int ind=0;
    for(j=0;j<h;j++)
    {
        data2 = data+(h-1-j)*lineBytes;
        pos=0;
        for(i=0;i<w;i++)
        {
            data2[pos]=(unsigned char)(b[ind]*255/maxVal);
            pos++;
            data2[pos]=(unsigned char)(g[ind]*255/maxVal);
            pos++;
            data2[pos]=(unsigned char)(r[ind]*255/maxVal);
            pos++;
            ind++;
        }
    }
    FILE* fp;
    fp = fopen(fname, "wb");
    fwrite(header, 1, 54, fp);
    fwrite(data, 1, lineBytes*h, fp);
    fclose(fp);
    delete []data;
    return 0;
}



template <class T>
int convert3aSta_romeHw_yrgb(void* buf, int w, int h, T* y, T* r, T* g, T* b)
{
    int i;
    int j;
    unsigned char* py;
    unsigned char* prgb;
    int ind=0;
    for(j=0;j<h;j++)
    {
        prgb = ((unsigned char*)buf)+5*w*j;
        py = ((unsigned char*)buf)+(5*w)*j+4*w;
        for(i=0;i<w;i++)
        {
            r[ind]=*prgb;
            g[ind]=*(prgb+1);
            b[ind]=*(prgb+2);
            y[ind]=*py;
            py++;
            prgb+=4;
            ind++;
        }
    }
    return 0;
}

int convert3aSta_romeHw_bmp(void* arr, int w, int h, const char* aeF, const char* awbF);




int cal_center_3aSta_yrgb_romeHw(void* buf, int w, int h, double* yrgb);

template <class T>
int convert3aSta_romeHw_y(void* buf, int w, int h, T* y, int gain)
{
    int i;
    int j;
    unsigned char* py;
    int ind=0;
    for(j=0;j<h;j++)
    {
        py = ((unsigned char*)buf)+(5*w)*j+4*w;
        for(i=0;i<w;i++)
        {
            y[ind]=(*py)*gain;
            py++;
            ind++;
        }
    }


    return 0;
}


class MemBuf
{
public:
    MemBuf();
    ~MemBuf();
    float* newFloat(int sz);
    double* newDouble(int sz);
    char* newChar(int sz);
    short* newShort(int sz);
    int* newInt(int sz);
    void deleteBuf();
private:
    void* buf;
};
