#ifndef ys_key
#define ys_key
#include <iostream>
#include <stdio.h>
#include <arpa/inet.h>
#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/objects.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#define ECDH_SIZE 33
namespace yrssf{
  class Signer{
    EC_KEY   * key;
    EC_POINT * point;
    EC_GROUP * group;
    public:
    unsigned char pubkey[ECDH_SIZE];
    char     * privkey;
    Signer   * next;
    bool init(){
      //Generate Public
      key = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);//NID_secp521r1
      
      EC_KEY_generate_key(key);
      
      point = (struct ec_point_st *)EC_KEY_get0_public_key(key);
      group = (struct ec_group_st *)EC_KEY_get0_group(key);
      
      privkey=BN_bn2hex(EC_KEY_get0_private_key(key));
      if(0 == (EC_POINT_point2oct(group, point, POINT_CONVERSION_COMPRESSED, pubkey, ECDH_SIZE, NULL))) return 0;
      return 1;
    }
    bool init(const EC_POINT *pbk,const BIGNUM * pri){
      key = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);//NID_secp521r1
      
      EC_KEY_set_private_key(key,pri);
      EC_KEY_set_public_key(key,pbk);
      
      point = (struct ec_point_st *)EC_KEY_get0_public_key(key);
      group = (struct ec_group_st *)EC_KEY_get0_group(key);
      
      privkey=BN_bn2hex(EC_KEY_get0_private_key(key));
      if(0 == (EC_POINT_point2oct(group, point, POINT_CONVERSION_COMPRESSED, pubkey, ECDH_SIZE, NULL))) return 0;
      return 1;
    }
    bool init(const char * pvk,const char * pbk){
      EC_POINT * ppk=EC_POINT_new(group);
      BIGNUM   * vpk=BN_new();
      
      BN_hex2bn(&vpk,pvk);
      EC_POINT_oct2point(group,ppk,(unsigned char*)pbk,ECDH_SIZE,NULL);
      
      bool res=init(ppk,vpk);
      EC_POINT_free(ppk);
      BN_free(vpk);
      return res;
    }
    bool init(const EC_POINT * pbk){
      key = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);//NID_secp521r1
      
      EC_KEY_set_public_key(key,pbk);
      
      point = (struct ec_point_st *)EC_KEY_get0_public_key(key);
      group = (struct ec_group_st *)EC_KEY_get0_group(key);
      
      if(0 == (EC_POINT_point2oct(group, point, POINT_CONVERSION_COMPRESSED, pubkey, ECDH_SIZE, NULL))) return 0;
      return 1;
    }
    bool init(const char * pbk){
      EC_POINT * ppk=EC_POINT_new(group);
      EC_POINT_oct2point(group,ppk,(unsigned char*)pbk,ECDH_SIZE,NULL);
      bool res=init(ppk);
      EC_POINT_free(ppk);
      return res;
    }
    bool sign(unsigned char * data,unsigned int datalen,unsigned char * signature,unsigned int * sig_len){
      return 1==ECDSA_sign(0,data,datalen,signature,sig_len,key);
    }
    bool check(unsigned char * data,unsigned int datalen,unsigned char * signature,unsigned int sig_len){
      if(ECDSA_verify(0,data,20,signature,sig_len,key)==1){
        return 1;
      }else{
        if(next)
          return next->check(data,datalen,signature,sig_len);
        else
          return 0;
      }
    }
    Signer(){
      key=NULL;
      next=NULL;
      privkey=NULL;
      init();
    }
    ~Signer(){
      if(key)    EC_KEY_free(key);
      if(next)   delete next;
      if(privkey)OPENSSL_free(privkey);
    }
  }signer;
  class Key{
    EC_KEY   *ecdh;
    EC_POINT *point;
    EC_GROUP *group;
    public:
    struct netSendkey{
      uint32_t      siglen;
      unsigned char sign  [1024];
      unsigned char pubkey[ECDH_SIZE];
    }*buf;
    unsigned char pubkey[ECDH_SIZE];
    unsigned char shared[ECDH_SIZE];
    int len;
    bool init(){
      //ecdh = EC_KEY_new();
      //Generate Public
      ecdh = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);//NID_secp521r1
      EC_KEY_generate_key(ecdh);
      point = (struct ec_point_st *)EC_KEY_get0_public_key(ecdh);
      group = (struct ec_group_st *)EC_KEY_get0_group(ecdh);
      if(0 == (len = EC_POINT_point2oct(group, point, POINT_CONVERSION_COMPRESSED, pubkey, ECDH_SIZE, NULL))) return 0;
      return 1;
    }
    bool computekey(unsigned char pubkey2[]){
      EC_POINT *point2c;
      point2c = EC_POINT_new(group);
      EC_POINT_oct2point(group, point2c, pubkey2, ECDH_SIZE, NULL);
      //if (0 != EC_POINT_cmp(group, point2, point2c, NULL)) return 0;
      if(0 == (len = ECDH_compute_key(shared, ECDH_SIZE, point2c, ecdh, NULL))){
        EC_POINT_free(point2c);
        return 0;
      }else{
        EC_POINT_free(point2c);
        return 1;
      }
    }
    void initbuf(){
      int i;
      init();
      for(i=0;i<ECDH_SIZE;i++){
        buf->pubkey[i]=pubkey[i];
      }
    }
    void computekey(){
      computekey(buf->pubkey);
    }
    bool checksign(){
      unsigned int len=ntohl(buf->siglen);
      if(len>1024)return 0;
      return signer.check(buf->pubkey,ECDH_SIZE,buf->sign,len);
    }
    void sign(){
      unsigned int len;
      signer.sign(buf->pubkey,ECDH_SIZE,buf->sign,&len);
      buf->siglen=htonl(len);
    }
    Key(){
      ecdh=NULL;
    }
    ~Key(){
      if(ecdh)   EC_KEY_free(ecdh);
    }
  };
}
#endif