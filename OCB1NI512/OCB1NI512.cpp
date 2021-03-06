
#include <wmmintrin.h>
#include <immintrin.h>
#include <emmintrin.h>
#include <iostream>
#include <cstring>
#include <math.h>  
using namespace std;

typedef __m512i block512;

typedef __m128i block128; 

void calculateChecksum(block512 * checksum, block128 *Stag, unsigned char *key128, block512 tempNonce,int length,int indiceNonce, int number_of_rounds);

void calculateAssociatedData(const unsigned char *ad, unsigned char *nsec, long adLength, block512*keys,unsigned char *key128, block128 *Stag, int number_of_rounds);

void key128tokey512(unsigned char Expandkey128[176], unsigned char Expandkey512[704]);

static inline block512 X16_block512(block512 bl);

static inline block128 double_block(block128 bl);

static inline void CalculateNonceBlock(block512 *nonce512, unsigned char *nonce, unsigned char *key, unsigned rounds, int size);

static inline void AES_ecb128_encrypt_blks(block128 *in, block128 *out, unsigned nblks, unsigned char *key, unsigned rounds);

static inline void getDelta(block128 *delta, unsigned char *nonce, unsigned char *key, unsigned rounds, int deltalen);

static inline void AES_ecb128_decrypt_blks(block128 *in, block128 *out, unsigned nblks, block128 *keys, unsigned rounds);

static inline void AES_ecb_decrypt_blks(block512 *blks, block512 *out, unsigned nblks,  block512 *key, unsigned rounds);

void imprimiArreglo(int tam, unsigned char *in );

void imprimirM128(block128 tmp);

#define swap_if_le(b) \
      _mm_shuffle_epi8(b,_mm_set_epi8(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15))//_mm512_shuffle_epi8 _mm512_set_epi8

#define swap_if_le512(b) \
      _mm512_shuffle_epi8(b,_mm512_set_epi8(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15, 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15, 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15, 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15))//_mm512_shuffle_epi8 _mm512_set_epi8

static inline unsigned ntz(unsigned x) {
		static const unsigned char tz_table[32] =
		{ 0,  1, 28,  2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17,  4, 8,
		 31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18,  6, 11,  5, 10, 9};
		return tz_table[((uint32_t)((x & -x) * 0x077CB531u)) >> 27];
	}

inline __m128i AES_128_ASSIST (__m128i temp1, __m128i temp2)
{
    __m128i temp3;
    temp2 = _mm_shuffle_epi32 (temp2 ,0xff);
    temp3 = _mm_slli_si128 (temp1, 0x4);
    temp1 = _mm_xor_si128 (temp1, temp3);
    temp3 = _mm_slli_si128 (temp3, 0x4);
    temp1 = _mm_xor_si128 (temp1, temp3);
    temp3 = _mm_slli_si128 (temp3, 0x4);
    temp1 = _mm_xor_si128 (temp1, temp3);
    temp1 = _mm_xor_si128 (temp1, temp2);
    return temp1;
}
void AES_128_Key_Expansion (unsigned char *userkey, unsigned char *key)
{
    __m128i temp1, temp2;
    __m128i *Key_Schedule = (__m128i*)key;
    temp1 = _mm_loadu_si128((__m128i*)userkey);
    Key_Schedule[0] = temp1;
    temp2 = _mm_aeskeygenassist_si128 (temp1 ,0x1);
    temp1 = AES_128_ASSIST(temp1, temp2);
    Key_Schedule[1] = temp1;
    temp2 = _mm_aeskeygenassist_si128 (temp1,0x2);
    temp1 = AES_128_ASSIST(temp1, temp2);
    Key_Schedule[2] = temp1;
    temp2 = _mm_aeskeygenassist_si128 (temp1,0x4);
    temp1 = AES_128_ASSIST(temp1, temp2);
    Key_Schedule[3] = temp1;
    temp2 = _mm_aeskeygenassist_si128 (temp1,0x8);
    temp1 = AES_128_ASSIST(temp1, temp2);
    Key_Schedule[4] = temp1;
    temp2 = _mm_aeskeygenassist_si128 (temp1,0x10);
    temp1 = AES_128_ASSIST(temp1, temp2);
    Key_Schedule[5] = temp1;
    temp2 = _mm_aeskeygenassist_si128 (temp1,0x20);
    temp1 = AES_128_ASSIST(temp1, temp2);
    Key_Schedule[6] = temp1;
    temp2 = _mm_aeskeygenassist_si128 (temp1,0x40);
    temp1 = AES_128_ASSIST(temp1, temp2);
    Key_Schedule[7] = temp1;
    temp2 = _mm_aeskeygenassist_si128 (temp1,0x80);
    temp1 = AES_128_ASSIST(temp1, temp2);
    Key_Schedule[8] = temp1;
    temp2 = _mm_aeskeygenassist_si128 (temp1,0x1b);
    temp1 = AES_128_ASSIST(temp1, temp2);
    Key_Schedule[9] = temp1;
    temp2 = _mm_aeskeygenassist_si128 (temp1,0x36);
    temp1 = AES_128_ASSIST(temp1, temp2);
    Key_Schedule[10] = temp1;
}
void AES_128_Key_Expansion_Decrypt (__m128i * key128Encrypt, __m128i * key128Decrypt)
{
    key128Decrypt[0] = key128Encrypt[10];
    key128Decrypt[1] = _mm_aesimc_si128(key128Encrypt[9]);
	key128Decrypt[2] = _mm_aesimc_si128(key128Encrypt[8]);
	key128Decrypt[3] = _mm_aesimc_si128(key128Encrypt[7]);
	key128Decrypt[4] = _mm_aesimc_si128(key128Encrypt[6]);
	key128Decrypt[5] = _mm_aesimc_si128(key128Encrypt[5]);
	key128Decrypt[6] = _mm_aesimc_si128(key128Encrypt[4]);
	key128Decrypt[7] = _mm_aesimc_si128(key128Encrypt[3]);
	key128Decrypt[8] = _mm_aesimc_si128(key128Encrypt[2]);
	key128Decrypt[9] = _mm_aesimc_si128(key128Encrypt[1]);
    key128Decrypt[10] = key128Encrypt[0];

}
void imprimirM128(block128 tmp){
    unsigned char impresion[16] = {0};
    _mm_store_si128 ((__m128i*)impresion,tmp);
    imprimiArreglo(16,&impresion[0]);
    printf("\n---------------------------\n");
}

void imprimiArreglo(int tam, unsigned char *in )
{

    for (int i = 0; i<tam; i++){
        printf("%02x", in[i] );
    }
}
void imprimiArreglo(int tam, const unsigned char *in ){

    for (int i = 0; i<tam; i++){
        printf("%02x", in[i] );
    }
}
void print512_num(__m512i var)
{
    unsigned int val[32];
    memcpy(val, &var, sizeof(val));

    for (int i = 0; i<32; i++){
        printf("%x ", val[i] );
    }
    cout<<endl;
}


static inline void AES_ecb_encrypt_blks(
block512 *blks, //bloques de entrada 
block512 *out, //bloques de salida
unsigned nblks, //numero de bloques 
block512 *key, //bloque de las llaves
unsigned rounds)//numero de rondas 
{
    unsigned i,j,rnds=rounds;
	for (i=0; i<nblks; ++i)
	    out[i] =_mm512_xor_si512(blks[i], key[0]);
	for(j=1; j<rnds; ++j)
	    for (i=0; i<nblks; ++i)
		    out[i] = _mm512_aesenc_epi128(out[i], key[j]);
	for (i=0; i<nblks; ++i)
	    out[i] =_mm512_aesenclast_epi128(out[i], key[j]);
}
static inline void AES_ecb_decrypt_blks(
block512 *blks, //bloques de entrada 
block512 *out, //bloques de salida
unsigned nblks, //numero de bloques 
block512 *key, //bloque de las llaves
unsigned rounds)//numero de rondas 
{
    unsigned i,j,rnds=rounds;
	for (i=0; i<nblks; ++i)
	    out[i] =_mm512_xor_si512(blks[i], key[0]);
	for(j=1; j<rnds; ++j)
	    for (i=0; i<nblks; ++i)
		    out[i] = _mm512_aesdec_epi128(out[i], key[j]);
	for (i=0; i<nblks; ++i)
	    out[i] =_mm512_aesdeclast_epi128(out[i], key[j]);
}
static inline void AES_ecb128_encrypt_blks(block128 *in, block128 *out, unsigned nblks, unsigned char *key, unsigned rounds)
{
	unsigned i,j,rnds=rounds;
    const __m128i *sched = ((__m128i *)(key));
	for (i=0; i<nblks; ++i)
	    out[i] =_mm_xor_si128(in[i], sched[0]);
	for(j=1; j<rnds; ++j)
	    for (i=0; i<nblks; ++i)
		    out[i] = _mm_aesenc_si128(out[i], sched[j]);
	for (i=0; i<nblks; ++i)
	    out[i] =_mm_aesenclast_si128(out[i], sched[j]);
}

static inline void AES_ecb128_decrypt_blks(block128 *in, block128 *out, unsigned nblks, block128 *keys, unsigned rounds)
{
    unsigned i,j,rnds=rounds;
	for (i=0; i<nblks; ++i)
	    out[i] =_mm_xor_si128(in[i], keys[0]);
    for(j=1; j<rnds; ++j)
	    for (i=0; i<nblks; ++i)
		    out[i] = _mm_aesdec_si128(out[i], keys[j]);
	for (i=0; i<nblks; ++i)
	    out[i] =_mm_aesdeclast_si128(out[i], keys[j]);
    
   
    
}

static inline block128 AES_encrypt(block128 in, unsigned char *key, unsigned rounds)
{
	int j,rnds=rounds;
	const __m128i *sched = ((__m128i *)(key));
	__m128i tmp = in;
	tmp = _mm_xor_si128 (tmp,sched[0]);
	for (j=1; j<rnds; j++)  tmp = _mm_aesenc_si128 (tmp,sched[j]);
	tmp = _mm_aesenclast_si128 (tmp,sched[j]);
	return tmp; 
}


static inline block128 double_block(block128 bl) 
{
		const __m128i mask = _mm_set_epi32(135,1,1,1);//__m512i _mm512_set_epi32 
		__m128i tmp = _mm_srai_epi32(bl, 31);//_mm512_srai_epi32
        
		tmp = _mm_and_si128(tmp, mask); //_mm512_and_si512
        tmp = _mm_shuffle_epi32(tmp, _MM_SHUFFLE(2,1,0,3));//_mm512_shuffle_epi32
		bl = _mm_slli_epi32(bl, 1);//_mm512_slli_epi32

		return _mm_xor_si128(bl,tmp); //_mm512_xor_si512
	}




static void
shift_right(unsigned char *x)
/* 128-bit shift-right by 1 bit:  *x >>= 1                               */
{
    int i;
    for (i = 15; i > 0; i--) {
        x[i] = (x[i] >> 1) | (x[i-1] & 1 ? 0x80u : 0);
    }
    x[0] = (x[0] >> 1);
}


static inline block128 invert_block(block128 bl) 
{       
        //  bl = swap_if_le(bl)
		// const __m128i mask = _mm_set_epi32(135,1,1,1);//__m512i _mm512_set_epi32 
		// __m128i tmp = _mm_srai_epi32(bl, 31);//_mm512_srai_epi32
		// tmp = _mm_and_si128(tmp, mask); //_mm512_and_si512
		// tmp = _mm_shuffle_epi32(tmp, _MM_SHUFFLE(2,1,0,3));//_mm512_shuffle_epi32
		
        // bl = _mm_srli_epi32(bl, 1);//_mm512_slli_epi32
		// _mm_xor_si128(bl,tmp); //_mm512_xor_si512


        unsigned char tmp[16]={0};
        _mm_storeu_si128(&((__m128i*)tmp)[0],bl);

         unsigned last_bit;
        last_bit = tmp[15] & 0x01;
        shift_right(tmp);
        if (last_bit) {
            tmp[0] ^= 0x80;
            tmp[15] ^= 0x43;
        }
        bl = _mm_loadu_si128(&((__m128i*)tmp)[0]);

        return bl;
	}

static inline block512 X16_block512(block512 bl) 
{
		const __m512i mask = _mm512_set_epi32(135,1,1,1, 135,1,1,1, 135,1,1,1, 135,1,1,1);//__m512i _mm512_set_epi32 
		__m512i tmp = _mm512_srai_epi32(bl, 31);//_mm512_srai_epi32
		
        tmp = _mm512_and_si512(tmp, mask); //_mm512_and_si512
		
        tmp = _mm512_shuffle_epi32(tmp, _MM_SHUFFLE(2,1,0,3));//_mm512_shuffle_epi32
        
        bl = _mm512_slli_epi32(bl, 1);//_mm512_slli_epi32

        bl = _mm512_xor_si512(bl,tmp);
        

        tmp = _mm512_srai_epi32(bl, 31);//_mm512_srai_epi32
		
        tmp = _mm512_and_si512(tmp, mask); //_mm512_and_si512
		
        tmp = _mm512_shuffle_epi32(tmp, _MM_SHUFFLE(2,1,0,3));//_mm512_shuffle_epi32
        
        bl = _mm512_slli_epi32(bl, 1);//_mm512_slli_epi32

        bl = _mm512_xor_si512(bl,tmp);


        tmp = _mm512_srai_epi32(bl, 31);//_mm512_srai_epi32 
		
        tmp = _mm512_and_si512(tmp, mask); //_mm512_and_si512
		
        tmp = _mm512_shuffle_epi32(tmp, _MM_SHUFFLE(2,1,0,3));//_mm512_shuffle_epi32
        
        bl = _mm512_slli_epi32(bl, 1);//_mm512_slli_epi32

        bl = _mm512_xor_si512(bl,tmp);
        

        tmp = _mm512_srai_epi32(bl, 31);//_mm512_srai_epi32
		
        tmp = _mm512_and_si512(tmp, mask); //_mm512_and_si512
		
        tmp = _mm512_shuffle_epi32(tmp, _MM_SHUFFLE(2,1,0,3));//_mm512_shuffle_epi32
        
        bl = _mm512_slli_epi32(bl, 1);//_mm512_slli_epi32

        return _mm512_xor_si512(bl,tmp); //_mm512_xor_si512
	}
static inline void CalculateNonceBlock(block512 *nonce512, unsigned char *nonce, unsigned char *key, unsigned rounds, int size)
{
    int deltalen = 0;
    if(size%16 == 0)
        deltalen = size/16 ;
    else
        deltalen = (size + (16 -size%16) ) /16  ;
    
    __m128i delta[deltalen+1]; 

    __m128i zero = _mm_setzero_si128(); 

    getDelta(delta, nonce, key,  rounds, deltalen);

    for(int i = 0; i<= deltalen/4; i++ ){
        
        if( (i*4) < deltalen )
            nonce512[i] = _mm512_castsi128_si512( delta[(i*4) ] );
        else
            nonce512[i] = _mm512_castsi128_si512( zero);

        if( (i*4)+1 < deltalen )
            nonce512[i] = _mm512_inserti64x2(nonce512[i], delta[(i*4)+1], 1 );
        else
            nonce512[i] = _mm512_inserti64x2(nonce512[i], zero, 1 );

        if( (i*4)+2 < deltalen )
            nonce512[i] = _mm512_inserti64x2(nonce512[i], delta[(i*4)+2], 2 );
        else
            nonce512[i] = _mm512_inserti64x2(nonce512[i], zero, 2 );
        
        if( (i*4)+3 < deltalen )        
            nonce512[i] = _mm512_inserti64x2(nonce512[i], delta[(i*4)+3], 3 );
        else
            nonce512[i] = _mm512_inserti64x2(nonce512[i], zero, 3 );


        // unsigned char c[64] = {0};
        // _mm512_storeu_si512(&((__m512i*)c)[0],nonce512[i]);
        // cout<<"checando   \n";
        // imprimiArreglo(16,&c[0]);
        // printf("\n---------------------------");
        // cout<<endl;
        // imprimiArreglo(16,&c[16]);
        // printf("\n---------------------------");
        // cout<<endl;
        // imprimiArreglo(16,&c[32]);
        // printf("\n---------------------------");
        // cout<<endl;
        // imprimiArreglo(16,&c[48]);
        // printf("\n---------------------------");
        // cout<<endl;
    }

    

  
}

static inline void CalculateLtable(block128 *L_Table, block128 *L_inverted, unsigned char *nonce, unsigned char *key, unsigned rounds, int size)
{
    // __m128i tmp = _mm_loadu_si128(&((__m128i*)nonce)[0]);
    __m128i tmp = _mm_setzero_si128();
    tmp = AES_encrypt(tmp, key, rounds); 
    L_Table[0] = tmp;
    tmp = swap_if_le(tmp);
    for (int i = 1; i < size; i++) {
		tmp = double_block(tmp);
    	L_Table[i] = swap_if_le(tmp);
    }
    L_inverted[0] = invert_block(L_Table[0]);
}


static inline void getDelta(block128 *delta, unsigned char *nonce, unsigned char *key, unsigned rounds, int deltalen)
{
    unsigned char impresion[64]= {0}; 
    int PRE_COMP_BLOCKS = 31;
    __m128i nonce128 = _mm_loadu_si128(&((__m128i*)nonce)[0]);
    __m128i L_Table[PRE_COMP_BLOCKS];
    __m128i L_inverted;
    
    CalculateLtable(L_Table,&L_inverted, nonce, key, rounds,PRE_COMP_BLOCKS);
    
    delta[0] = _mm_xor_si128(L_Table[0],nonce128);

    delta[0] = AES_encrypt(delta[0], key, rounds);

    for (int i=1; i<deltalen; i++){
        delta[i] = _mm_xor_si128(L_Table[ntz(i)],delta[i-1]);
    }
    delta[deltalen-2] = _mm_xor_si128(L_inverted,delta[deltalen-1]);
    
    
}

void AES_OCB512_encrypt(const unsigned char *in,//pointer to the PLAINTEXT 
unsigned char *out, //pointer to the CIPHERTEXT buffer
const unsigned char *ad,
long adLength,
long length,//text length in bytes
unsigned char *key,//pointer to the expanded key schedule
unsigned char *nsec,//pointer to the NONCE
int number_of_rounds) //number of AES rounds 10,12 or 14
{
    __m512i tmp[4];
    __m512i checksum[1]; 
    __m128i Stag[1];
    __m512i nonce;
    __m512i tempNonce;
    __m512i cipherNonceTemp[4];
    
    __m512i keys[number_of_rounds+1];
    nonce = _mm512_setzero_epi32();
    checksum[0]= _mm512_setzero_epi32();
    Stag[0] = _mm_setzero_si128();
    unsigned char Expandkey512[704]={0};
    unsigned char key128[176];//pointer to the expanded key schedule for 128

    for (int i = 0; i<4; i++){
        tmp[i]= _mm512_setzero_epi32();
        cipherNonceTemp[i]= _mm512_setzero_epi32();
    }

    AES_128_Key_Expansion (key, key128);
    key128tokey512(key128, Expandkey512);

    int i,j;
    int tempLength=length;
    if(length%64 == 0)
        tempLength = length/64;
    else
        tempLength = (length + (64 -length%64) ) /64 ;
    
    for(i=0; i < number_of_rounds+1; i++){
        keys[i] = _mm512_loadu_si512(&((__m512i*)Expandkey512)[i]);
    }
    
    __m512i nonce512[tempLength+1];
    
    CalculateNonceBlock(nonce512, nsec, key128, number_of_rounds, length+16);

    
    
    i=0;
    while(tempLength>4){
        unsigned blocks = 0;
        //obtencion de bloques
        for(j = 0; j<4; j++ ){
            tmp[j] = _mm512_loadu_si512(&((__m512i*)in)[i+j]);
            blocks++;
        }
        //Actualizxacioon del checksum
        for(j = 0; j<blocks; j++ ){
            checksum[0] = _mm512_xor_si512(checksum[0], tmp[j]);
        }
        
        //xor del nonce con el mensaje correspondiente
        for(j=0; j < blocks; j++){
            tmp[j] = _mm512_xor_si512(nonce512[i+j], tmp[j]);
        }

        //cifrado del mensaje 
        AES_ecb_encrypt_blks(tmp,tmp, blocks, keys, number_of_rounds);
        
        //xort del mensaje cifrado con el nonce
        for(j=0; j < blocks; j++){
            tmp[j] = _mm512_xor_si512(nonce512[i+j], tmp[j]);
        }

        //Carga del mensaje cifrado a la salida
        for(j=0; j < blocks; j++){
            _mm512_storeu_si512(&((__m512i*)out)[i+j],tmp[j]);
        }

        //actualizacion de indices
        i = i+4;
        tempLength = tempLength-4;
    }
    
    //final block
    for(j = 0; j<tempLength-1; j++ ){
        tmp[j] = _mm512_loadu_si512(&((__m512i*)in)[i+j]);
    }
    for(j = 0; j<tempLength-1; j++ ){
        checksum[0] = _mm512_xor_si512(checksum[0], tmp[j]);
    }

    //xor del nonce con el mensaje correspondiente
    for(j=0; j < tempLength-1; j++){
        tmp[j] = _mm512_xor_si512(nonce512[i+j], tmp[j]);
    }
    
    //cifrado del mensaje 
    AES_ecb_encrypt_blks(tmp,tmp, tempLength-1, keys, number_of_rounds);
    
    //xort del mensaje cifrado con el nonce
    for(j=0; j < tempLength-1; j++){
        tmp[j] = _mm512_xor_si512(nonce512[i+j], tmp[j]);
    }

    //Carga del mensaje cifrado a la salida
    for(j=0; j < tempLength-1; j++){
        _mm512_storeu_si512(&((__m512i*)out)[i+j],tmp[j]);
    }
    
    

    // ultimo bloque de 512

    int finalBlocklenght = 0;
    int sizefinalBlockArray = 0;
    int indiceNonce = 0;
    int slice = 0;
    //comprobamos si es completo o no
    if(length%16 == 0){
        finalBlocklenght = length%64;
        if(finalBlocklenght == 0)
            finalBlocklenght = 64;
        sizefinalBlockArray = finalBlocklenght/16;
        slice = 16;
    }
    else{
        finalBlocklenght = length%64;
        //calculo de los bloquyes de 128 que necesitamos
        sizefinalBlockArray = (finalBlocklenght + (16 -finalBlocklenght%16) ) /16 ;
        
        slice = finalBlocklenght%16;
    }

    tempNonce = nonce512[i + tempLength-1];

    indiceNonce = sizefinalBlockArray; 
    if(sizefinalBlockArray>3){
        indiceNonce=0;
        tempNonce = nonce512[i + tempLength];
    }


    unsigned char finalblockChar[64] = {0};
    unsigned char temporal[16] = {0};

    temporal[15] = (slice << 3);

    for(j = 0; j<finalBlocklenght; j++){
        finalblockChar[j] = in[(length-finalBlocklenght )+ j ]; 
    }

    

    __m128i finalBlock[ sizefinalBlockArray ];
    __m128i delta128[ sizefinalBlockArray ];
    __m128i tmpfinalBlock[ sizefinalBlockArray ];
    __m128i bitlength; 
    __m128i temporal128;
    cipherNonceTemp[0] = nonce512[i + tempLength-1];
    
    for(j=0; j<sizefinalBlockArray; j++){
        finalBlock[j] =  _mm_loadu_si128(&((__m128i*)finalblockChar)[j]);
        delta128[j] =  _mm_loadu_si128(&((__m128i*)cipherNonceTemp)[j]);
    }


    bitlength = _mm_load_si128(&((__m128i*)temporal)[0]);

    for(j=0; j<sizefinalBlockArray; j++){
        tmpfinalBlock[j] = _mm_xor_si128(delta128[j],finalBlock[j]);
    }



    tmpfinalBlock[sizefinalBlockArray-1] = _mm_xor_si128(delta128[sizefinalBlockArray-1],bitlength);

    

    AES_ecb128_encrypt_blks(tmpfinalBlock, tmpfinalBlock, sizefinalBlockArray, key128 , 10);
    

    for(j=0; j<sizefinalBlockArray-1; j++){
        tmpfinalBlock[j] = _mm_xor_si128(delta128[j],tmpfinalBlock[j]);
    }


    temporal128 = tmpfinalBlock[sizefinalBlockArray-1];
    tmpfinalBlock[sizefinalBlockArray-1] = _mm_xor_si128(tmpfinalBlock[sizefinalBlockArray-1],finalBlock[sizefinalBlockArray-1]);

    if(length%16!=0)
        i = ( (length + (16 -length%16) ) /16 ) - sizefinalBlockArray;
    else
        i = ( (length) /16 ) - sizefinalBlockArray;
    

    for(j=0; j < sizefinalBlockArray; j++){
        _mm_storeu_si128(&((__m128i*)out)[i+j],tmpfinalBlock[j]);
    }
    
    if(finalBlocklenght%16 != 0){

        for(int j = 0; j<finalBlocklenght%16; j++){
            temporal128 = _mm_bsrli_si128(temporal128, 1 );
            // finalBlock[sizefinalBlockArray-1] = _mm_bslli_si128(finalBlock[sizefinalBlockArray-1], 1 );
        }
        for(int j = 0; j<finalBlocklenght%16; j++){
            // finalBlock[sizefinalBlockArray-1] = _mm_bslli_si128(finalBlock[sizefinalBlockArray-1], 1 );
            temporal128 = _mm_bslli_si128(temporal128, 1 );
        }
        
        temporal128 = _mm_xor_si128(temporal128, finalBlock[sizefinalBlockArray-1]);

        _mm_storeu_si128(&((__m128i*)finalblockChar)[sizefinalBlockArray-1],temporal128);
    }

    tmp[0] = _mm512_loadu_si512(&((__m512i*)finalblockChar)[0]);
    checksum[0] = _mm512_xor_si512(checksum[0], tmp[0]);
    
    calculateAssociatedData(ad, nsec, adLength, keys, key128, Stag, number_of_rounds);

    calculateChecksum(checksum,Stag, key128, tempNonce, length,indiceNonce, number_of_rounds);
}

void AES_OCB512_decrypt(const unsigned char *in,//pointer to the PLAINTEXT 
unsigned char *out, //pointer to the CIPHERTEXT buffer
const unsigned char *ad,
long adLength,
long length,//text length in bytes
unsigned char *key,//pointer to the expanded key schedule
unsigned char *nsec,//pointer to the NONCE
int number_of_rounds) //number of AES rounds 10,12 or 14
{
    __m512i tmp[4];
    __m512i checksum[1]; 
    __m128i Stag[1];
    __m512i nonce;
    __m512i tempNonce;
    __m512i cipherNonceTemp[4];
    __m512i keys512Decrypt[number_of_rounds+1];
    __m512i keys512Encrypt[number_of_rounds+1];
    
    __m512i keys[number_of_rounds+1];
    nonce = _mm512_setzero_epi32();
    checksum[0]= _mm512_setzero_epi32();
    Stag[0] = _mm_setzero_si128();
    unsigned char key128[176];//pointer to the expanded key schedule for 128
    unsigned char key128De[176];//pointer to the expanded key schedule for 128
    
    int i,j;
    int tempLength=length;

    for (int i = 0; i<4; i++){
        tmp[i]= _mm512_setzero_epi32();
        cipherNonceTemp[i]= _mm512_setzero_epi32();
    }

    AES_128_Key_Expansion (key, key128);
    __m128i *key128Encrypt  = ((__m128i *)(key128));
    __m128i key128Decrypt[number_of_rounds+1];

    AES_128_Key_Expansion_Decrypt(key128Encrypt,key128Decrypt);

    for(i=0; i < number_of_rounds+1; i++){
        keys512Decrypt[i] = _mm512_castsi128_si512( key128Decrypt[i] );
        keys512Decrypt[i] = _mm512_inserti64x2(keys512Decrypt[i], key128Decrypt[i], 1 );
        keys512Decrypt[i] = _mm512_inserti64x2(keys512Decrypt[i], key128Decrypt[i], 2 );
        keys512Decrypt[i] = _mm512_inserti64x2(keys512Decrypt[i], key128Decrypt[i], 3 );
        
        keys512Encrypt[i] = _mm512_castsi128_si512( key128Encrypt[i] );
        keys512Encrypt[i] = _mm512_inserti64x2(keys512Encrypt[i], key128Encrypt[i], 1 );
        keys512Encrypt[i] = _mm512_inserti64x2(keys512Encrypt[i], key128Encrypt[i], 2 );
        keys512Encrypt[i] = _mm512_inserti64x2(keys512Encrypt[i], key128Encrypt[i], 3 );
    }
    
    __m512i nonce512[tempLength+1];
    
    CalculateNonceBlock(nonce512, nsec, key128, number_of_rounds, length+16);

    
    if(length%64 == 0)
        tempLength = length/64;
    else
        tempLength = (length + (64 -length%64) ) /64 ;
    i=0;
    while(tempLength>4){
        unsigned blocks = 0;
        //obtencion de bloques
        for(j = 0; j<4; j++ ){
            tmp[j] = _mm512_loadu_si512(&((__m512i*)in)[i+j]);
            blocks++;
        }
        
        
        //xor del nonce con el mensaje correspondiente
        for(j=0; j < blocks; j++){
            tmp[j] = _mm512_xor_si512(nonce512[i+j], tmp[j]);
        }

        //cifrado del mensaje 
        AES_ecb_decrypt_blks(tmp,tmp, blocks, keys512Decrypt, number_of_rounds);
        
        //xort del mensaje cifrado con el nonce
        for(j=0; j < blocks; j++){
            tmp[j] = _mm512_xor_si512(nonce512[i+j], tmp[j]);
        }

        //Actualizxacioon del checksum
        for(j = 0; j<blocks; j++ ){
            checksum[0] = _mm512_xor_si512(checksum[0], tmp[j]);
        }

        //Carga del mensaje cifrado a la salida
        for(j=0; j < blocks; j++){
            _mm512_storeu_si512(&((__m512i*)out)[i+j],tmp[j]);
        }

        //actualizacion de indices
        i = i+4;
        tempLength = tempLength-4;
    }
    
    //final block
    for(j = 0; j<tempLength-1; j++ ){
        tmp[j] = _mm512_loadu_si512(&((__m512i*)in)[i+j]);
    }
    

    //xor del nonce con el mensaje correspondiente
    for(j=0; j < tempLength-1; j++){
        tmp[j] = _mm512_xor_si512(nonce512[i+j], tmp[j]);
    }
    
    //cifrado del mensaje 
    AES_ecb_decrypt_blks(tmp,tmp, tempLength-1, keys512Decrypt, number_of_rounds);
    
    //xort del mensaje cifrado con el nonce
    for(j=0; j < tempLength-1; j++){
        tmp[j] = _mm512_xor_si512(nonce512[i+j], tmp[j]);
    }

    for(j = 0; j<tempLength-1; j++ ){
        checksum[0] = _mm512_xor_si512(checksum[0], tmp[j]);
    }

    //Carga del mensaje cifrado a la salida
    for(j=0; j < tempLength-1; j++){
        _mm512_storeu_si512(&((__m512i*)out)[i+j],tmp[j]);
    }
    
    // ultimo bloque de 512

    int finalBlocklenght = 0;
    int sizefinalBlockArray = 0;
    int indiceNonce = 0;
    int slice = 0;
    //comprobamos si es completo o no
    if(length%16 == 0){
        finalBlocklenght = length%64;
        if(finalBlocklenght == 0)
            finalBlocklenght = 64;
        sizefinalBlockArray = finalBlocklenght/16;
        slice = 16;
    }
    else{
        finalBlocklenght = length%64;
        //calculo de los bloquyes de 128 que necesitamos
        sizefinalBlockArray = (finalBlocklenght + (16 -finalBlocklenght%16) ) /16 ;
        
        slice = finalBlocklenght%16;
    }

    tempNonce = nonce512[i + tempLength-1];

    indiceNonce = sizefinalBlockArray; 
    if(sizefinalBlockArray>3){
        indiceNonce=0;
        tempNonce = nonce512[i + tempLength];
    }


    unsigned char finalblockChar[64] = {0};
    unsigned char temporal[16] = {0};

    temporal[15] = (slice << 3);

    for(j = 0; j<finalBlocklenght; j++){
        finalblockChar[j] = in[(length-finalBlocklenght )+ j ]; 
    }

    

    __m128i finalBlock[ sizefinalBlockArray ];
    __m128i delta128[ sizefinalBlockArray ];
    __m128i tmpfinalBlock[ sizefinalBlockArray ];
    __m128i bitlength; 
    __m128i temporal128;
    cipherNonceTemp[0] = nonce512[i + tempLength-1];
    
    for(j=0; j<sizefinalBlockArray; j++){
        finalBlock[j] =  _mm_loadu_si128(&((__m128i*)finalblockChar)[j]);
        delta128[j] =  _mm_loadu_si128(&((__m128i*)cipherNonceTemp)[j]);

    }


    bitlength = _mm_load_si128(&((__m128i*)temporal)[0]);

    for(j=0; j<sizefinalBlockArray; j++){
        tmpfinalBlock[j] = _mm_xor_si128(delta128[j],finalBlock[j]);

    }
    tmpfinalBlock[sizefinalBlockArray-1] = _mm_xor_si128(delta128[sizefinalBlockArray-1],bitlength);

    AES_ecb128_decrypt_blks(tmpfinalBlock, tmpfinalBlock, sizefinalBlockArray-1, key128Decrypt , 10);
    
    
    AES_ecb128_encrypt_blks(&tmpfinalBlock[sizefinalBlockArray-1], &tmpfinalBlock[sizefinalBlockArray-1], 1, key128 , 10);
    

    for(j=0; j<sizefinalBlockArray-1; j++){
        tmpfinalBlock[j] = _mm_xor_si128(delta128[j],tmpfinalBlock[j]);
    }


    temporal128 = tmpfinalBlock[sizefinalBlockArray-1];
    tmpfinalBlock[sizefinalBlockArray-1] = _mm_xor_si128(tmpfinalBlock[sizefinalBlockArray-1],finalBlock[sizefinalBlockArray-1]);

    if(length%16!=0)
        i = ( (length + (16 -length%16) ) /16 ) - sizefinalBlockArray;
    else
        i = ( (length) /16 ) - sizefinalBlockArray;
    

    for(j=0; j < sizefinalBlockArray; j++){
        _mm_storeu_si128(&((__m128i*)out)[i+j],tmpfinalBlock[j]);
    }
    for(j=0; j < sizefinalBlockArray; j++){
        _mm_storeu_si128(&((__m128i*)finalblockChar)[j],tmpfinalBlock[j]);
       
    }
    if(finalBlocklenght%16 != 0){

        for(int j = 0; j<finalBlocklenght%16; j++){
            temporal128 = _mm_bsrli_si128(temporal128, 1 );
            // finalBlock[sizefinalBlockArray-1] = _mm_bslli_si128(finalBlock[sizefinalBlockArray-1], 1 );
        }
        for(int j = 0; j<finalBlocklenght%16; j++){
            // finalBlock[sizefinalBlockArray-1] = _mm_bslli_si128(finalBlock[sizefinalBlockArray-1], 1 );
            temporal128 = _mm_bslli_si128(temporal128, 1 );
        }
        
        temporal128 = _mm_xor_si128(temporal128, finalBlock[sizefinalBlockArray-1]);

        _mm_storeu_si128(&((__m128i*)finalblockChar)[sizefinalBlockArray-1],temporal128);
    }
    

    tmp[0] = _mm512_loadu_si512(&((__m512i*)finalblockChar)[0]);
    checksum[0] = _mm512_xor_si512(checksum[0], tmp[0]);
    
    // calculateAssociatedData(ad, nsec, adLength, keys, key128, Stag, number_of_rounds);
    calculateAssociatedData(ad, nsec, adLength, keys512Encrypt, key128, Stag, number_of_rounds);

    calculateChecksum(checksum,Stag, key128, tempNonce, length,indiceNonce, number_of_rounds);
}

void calculateAssociatedData(
    const unsigned char *ad, 
    unsigned char *nsec, 
    long adLength, 
    block512 *keys,
    unsigned char *key128, 
    block128 *Stag,
    int number_of_rounds
    )
{   
    __m512i tmp[4];
    __m512i checksum[1]; 
    __m512i nonce;
    __m512i cipherNonceTemp[4];
    
    nonce = _mm512_setzero_epi32();
    checksum[0]= _mm512_setzero_epi32();

    for (int i = 0; i<4; i++){
        tmp[i]= _mm512_setzero_epi32();
        cipherNonceTemp[i]= _mm512_setzero_epi32();
    }

    int i,j;
    int tempLength=adLength;
    if(adLength%64 == 0)
        tempLength = adLength/64;
    else
        tempLength = (adLength + (64 -adLength%64) ) /64 ;
    // nonce =  CalculateNonceBlock(nsec, key128, number_of_rounds);
    __m512i nonce512[tempLength+1];
    CalculateNonceBlock(nonce512, nsec, key128, number_of_rounds, adLength+16);

    i=0;
    if(adLength%64 == 0)
        tempLength = adLength/64;
    else
        tempLength = (adLength + (64 - adLength%64) ) /64 ;

    while(tempLength>4){
        unsigned blocks = 0;
        //obtencion de bloques
        for(j = 0; j<4; j++ ){
            tmp[j] = _mm512_loadu_si512(&((__m512i*)ad)[i+j]);
            blocks++;
        }
       
        //xor del nonce con el mensaje correspondiente
        for(j=0; j < blocks; j++){
            tmp[j] = _mm512_xor_si512(nonce512[i+j], tmp[j]);
        }

        //cifrado del mensaje 
        AES_ecb_encrypt_blks(tmp,tmp, blocks, keys, number_of_rounds);
        
        //Actualizxacioon del checksum
        for(j = 0; j<blocks; j++ ){
            checksum[0] = _mm512_xor_si512(checksum[0], tmp[j]);
        }
        
        //actualizacion de indices
        i = i+4;
        tempLength = tempLength-4;
    }

     //final block
    for(j = 0; j<tempLength-1; j++ ){
        tmp[j] = _mm512_loadu_si512(&((__m512i*)ad)[i+j]);
    }

    //xor del nonce con el mensaje correspondiente
    for(j=0; j < tempLength-1; j++){
        tmp[j] = _mm512_xor_si512(nonce512[i+j], tmp[j]);
    }
   
    //cifrado del mensaje 
    AES_ecb_encrypt_blks(tmp,tmp, tempLength-1, keys, number_of_rounds);

    for(j = 0; j<tempLength-1; j++ ){
        checksum[0] = _mm512_xor_si512(checksum[0], tmp[j]);
    }

    // ultimo bloque de 512

    int finalBlocklenght = 0;
    int sizefinalBlockArray = 0;
    //comprobamos si es completo o no
    if(adLength%16 == 0){
        finalBlocklenght = adLength%64;
        if(finalBlocklenght == 0)
            finalBlocklenght = 64;
        sizefinalBlockArray = finalBlocklenght/16;
    }
    else{
        finalBlocklenght = adLength%64;
        //calculo de los bloquyes de 128 que necesitamos
        sizefinalBlockArray = (finalBlocklenght + (16 -finalBlocklenght%16) ) /16 ;
    }

    unsigned char finalblockChar[64] = {0};

    for(j = 0; j<finalBlocklenght; j++){
        finalblockChar[j] = ad[(adLength-finalBlocklenght )+ j ]; 
    }
    if(finalBlocklenght%16 != 0){
        finalblockChar[finalBlocklenght] = 0; //poner 1 en vez de 0
    }

    __m128i finalBlock[ sizefinalBlockArray ];
    __m128i delta128[ sizefinalBlockArray ];
    __m128i tmpfinalBlock[ sizefinalBlockArray ];

    cipherNonceTemp[0] = nonce512[i + tempLength-1];;
    
    for(j=0; j<sizefinalBlockArray; j++){
        finalBlock[j] =  _mm_loadu_si128(&((__m128i*)finalblockChar)[j]);
        delta128[j] =  _mm_loadu_si128(&((__m128i*)cipherNonceTemp)[j]);
        
    }

    

    for(j=0; j<sizefinalBlockArray; j++){
        tmpfinalBlock[j] = _mm_xor_si128(delta128[j],finalBlock[j]);
    }
    AES_ecb128_encrypt_blks(tmpfinalBlock, tmpfinalBlock, sizefinalBlockArray, key128 , 10);
    


    __m128i checksumBlock[4];
    checksumBlock[0] =  _mm_loadu_si128(&((__m128i*)checksum)[0]);
    checksumBlock[1] =  _mm_loadu_si128(&((__m128i*)checksum)[1]);
    checksumBlock[2] =  _mm_loadu_si128(&((__m128i*)checksum)[2]);
    checksumBlock[3] =  _mm_loadu_si128(&((__m128i*)checksum)[3]);
    
    i=0;
        // //suma de las variables de checksum
    __m128i checksumFinal;
    checksumFinal = _mm_setzero_si128();

    for(j=0; j<sizefinalBlockArray; j++ ){
        checksumFinal = _mm_xor_si128( checksumFinal, tmpfinalBlock[j]); 
    }

    while(adLength>0 and i<4){
        checksumFinal =  _mm_xor_si128( checksumFinal, checksumBlock[i]);
        adLength = adLength-16;
        i++;
    }
    Stag[0] = checksumFinal;

}
void calculateChecksum(block512 *checksum, //pointer to the Checksum in 512 block
block128 *Stag, //resultado del checsksum de los datos asociados ya cifrados
unsigned char *key128, //llaves para 128 bits
block512 tempNonce, //nonce anterior al actual
int length,//tama??o del bloque
int indiceNonce, //indice of final part of block sum
int number_of_rounds) //numero de rondas
{
    int i=0;
    unsigned char impresion[16]={0};
    


    //divisiond el checksum para poder sumarlo
    __m128i checksumBlock[4];
    __m128i deltaChecksum;
    __m128i addIndice;
    checksumBlock[0] =  _mm_loadu_si128(&((__m128i*)checksum)[0]);
    checksumBlock[1] =  _mm_loadu_si128(&((__m128i*)checksum)[1]);
    checksumBlock[2] =  _mm_loadu_si128(&((__m128i*)checksum)[2]);
    checksumBlock[3] =  _mm_loadu_si128(&((__m128i*)checksum)[3]);

    deltaChecksum =   _mm_loadu_si128(&((__m128i*)&tempNonce)[indiceNonce]); 

    // //suma de las variables de checksum
    __m128i checksumFinal  = _mm_setzero_si128();
    i=0;
    while(length>0 and i<4){
        checksumFinal =  _mm_xor_si128( checksumFinal, checksumBlock[i]);
        length = length-16;
        i++;
    }

        imprimirM128(checksumFinal);
        //xor delta checksumn
        checksumFinal =  _mm_xor_si128( checksumFinal, deltaChecksum);

        //cifrado del checksum
        checksumFinal = AES_encrypt(checksumFinal, key128, number_of_rounds);

        checksumFinal = _mm_xor_si128(checksumFinal, Stag[0]); 
        //falta xor con el tag
        
        _mm_store_si128 ((__m128i*)impresion,Stag[0]);
        cout<<"S          ";
        imprimiArreglo(16,&impresion[0]);
        printf("\n---------------------------");
        cout<<endl;

        _mm_store_si128 ((__m128i*)impresion,checksumFinal);
        cout<<"Tag        ";
        imprimiArreglo(16,&impresion[0]);
        printf("\n---------------------------");
        cout<<endl;

        
}

void key128tokey512(unsigned char Expandkey128[176], unsigned char Expandkey512[704])
{
    
    int k=0;
    for(int i=0; i<176; i=i+16){
        for(int j=0; j<16; j++){    
            Expandkey512[(k*64)+j]=Expandkey128[i+j];
            Expandkey512[(k*64)+16+j]=Expandkey128[i+j];
            Expandkey512[(k*64)+32+j]=Expandkey128[i+j];
            Expandkey512[(k*64)+48+j]=Expandkey128[i+j];
        }
        k++;

    }
    
}
int main(){
    unsigned char k[16] = {
        0x00,0x01,0x02,0x03, 
        0x04,0x05,0x06,0x07,
        0x08,0x09,0x0a,0x0b,
        0x0c,0x0d,0x0e,0x0f
    };
    int number_of_rounds=10;
    unsigned char keys[176] = {0};
    unsigned char Expandkey512[704]={0};
    unsigned long long mlen=16;
    unsigned long long adlen=32;
    const unsigned char m[mlen] = {
        0x37, 0xdf, 0x8c, 0xe1, 
        0x5b, 0x48, 0x9b, 0xf3, 
        0x1d, 0x0f, 0xc4, 0x4d,
        0xa1, 0xfa, 0xf6, 0xd6,

        // 0x00,0x01,0x02,0x03, 
        // 0x04,0x05,0x06,0x07,
        // 0x08,0x09,0x0a,0x0b,
        // 0x0c,0x0d,0x0e,0x0f

    };

    const unsigned char ad[adlen] = {
        0x2b,0x28,0xab,0x09,
        0x7e,0xae,0xf7,0xcf,
        0x15,0xd2,0x15,0x4f,
        0x16,0xa6,0x88,0x3c,

        0x2b,0x28,0xab,0x09,
        0x7e,0xae,0xf7,0xcf,
        0x15,0xd2,0x15,0x4f,
        0x16,0xa6,0x88,0x3c,        
    };
    unsigned char nonce[16] = {
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x01,
    };
    unsigned char c[64]={0};

    

    // AES_OCB512_encrypt(m, c, ad, adlen, mlen, k, nonce, number_of_rounds);  
    AES_OCB512_decrypt(m, c, ad, adlen, mlen, k, nonce, number_of_rounds);  

    cout<<endl;
    cout<<"Key          ";
    imprimiArreglo(16,k);
    printf("\n---------------------------");
    cout<<endl;

    cout<<"Nonce        ";
    imprimiArreglo(16,nonce);
    printf("\n---------------------------");
    cout<<endl;

    cout<<"Plaintext    ";
    imprimiArreglo(mlen,m);
    printf("\n---------------------------");
    cout<<endl;

    cout<<"Ciphertext   ";
    imprimiArreglo(mlen,c);
    printf("\n---------------------------");
    cout<<endl;

    // for(int i = 0; i<64; i=i+16){
    // imprimiArreglo(16,&c[i]);
    // printf("\n---------------------------");
    // cout<<endl;
    // }
    // https://stackoverflow.com/questions/11116769/how-to-combine-two-m128-values-to-m256
   
    return 0;
}