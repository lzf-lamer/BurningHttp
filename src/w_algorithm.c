#include<stdlib.h>
#include<string.h>
#include<assert.h>
#include"w_algorithm.h"

/**
 * 高精度加法,n1,n2为两个到序数组,0位保存的是元素个数
 * 返回同参数的到序数组
 */
int * Add(int *n1,int *n2)
{
	int * re;
	int i,len;
	len = n1[0]>n2[0]?n1[0]:n2[0];
    re =(int *)malloc((len+1) * sizeof(int));
    memset(re,0,(len+1) * sizeof(int));
    re[0]=len;
	for(i=1;i<=len;i++)
	{
        re[i]+=n1[i]+n2[i];
        re[i+1]+=re[i]/10;
        re[i]=re[i]%10;
	}
    if(re[re[0]+1]>0) re[0]++;
    return re;
}
/**
 *  Base64解码
 */
void creat_array(unsigned char * array)
{
    int index = 0;
    for(index=0;index<123;index++)
    {
        if(index == 0x2B) // +
        {
            *(array + index) = 0x3E;
        }else if(index == 0x2F) // /
        {
            *(array + index) = 0x3F;
        }else if(index >= 0x30 && index <= 0x39) // 0-9
        {
            *(array + index) = 0x34 + index - 0x30;
        }else if(index >= 0x41 && index <= 0x5A) // A-Z
        {
            *(array + index) = index - 0x41;
        }else if(index >= 0x61 && index <=0x7A) // a-z
        {
            *(array + index) = 0x1A + index - 0x61;
        }else
        {
            *(array + index) = 0x00;
        }
    }
    *(array + 123) = '\0';
}
int decode_string(const unsigned char * src,unsigned char * dest,unsigned char * array,int index)
{
    assert(src);
    int step = 0;
    unsigned char one;
    unsigned char two;
    unsigned char thr;
    
    if(src[3] == '=' && src[2] == '=')
    {
        one = (array[src[0]] << 2) | (array[src[1]] >> 4);
        *(dest + index) = one;
        step = 1;
    }else if(src[3] == '=')
    {
        one = (array[src[0]] << 2) | (array[src[1]] >> 4);
        two = ((array[src[1]] & 0x0F) << 4) | (array[src[2]] >> 2);
        
        *(dest + index++) = one;
        *(dest + index) = two;
        
        step = 2;
    }else
    {
        one = (array[src[0]] << 2) | (array[src[1]] >> 4);
        two = ((array[src[1]] & 0x0F) << 4) | (array[src[2]] >> 2);
        thr = (array[src[2]] << 6) | array[src[3]];
        
        *(dest + index++) = one;
        *(dest + index++) = two;
        *(dest + index)   = thr;
        
        step = 3;
    }
    return step;
}
void base64_decode(const unsigned char * src,unsigned char *dest)
{
    assert(src);
    unsigned char array[124];
    memset(array,0x00,124);
    creat_array(array);
    
    int index = 0;
    while(*src)
    {
        int step = decode_string(src,dest,array,index);
        
        index+=step;
        src+=4;
    }
    *(dest + index) = '\0';
}

