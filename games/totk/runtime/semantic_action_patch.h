/* 1.0.0 Mac player-operation patch. Offsets/guards were established by the local
   TOTK input analysis (2026-10-22); no raw Npad/UI masks or game resources change.
   Code is installed once before RX sealing. Live edits only store one atomic word
   at data-page +64. Six veneers preserve x16/x17, SP, LR and NZCV. */
#pragma once
#include "runtime.h"
typedef struct { uint32_t offset,original;unsigned source,reg,set; } SemanticActionSite;
static const SemanticActionSite semantic_action_sites[]={
 {0x1ccfe28,0x52800036,0,22,1},{0x1ccfc60,0x321e02d6,1,22,0},{0x1ccfe38,0x321d02d6,2,22,0},
 {0x1ccfe90,0x52800035,0,21,1},{0x1ccfc68,0x321e02b5,1,21,0},{0x1ccfea0,0x321d02b5,2,21,0}
};
enum { SEMANTIC_ACTION_PADDING=512, SEMANTIC_ACTION_DATA=64, SEMANTIC_ACTION_STUB_BYTES=32 };
static inline uint32_t semantic_action_branch(uint64_t from,uint64_t to){return 0x14000000u|((uint32_t)(((int64_t)to-(int64_t)from)/4)&0x3ffffffu);}
static inline int semantic_action_install(Module *m){
 const uint64_t stub=((m->text_end+15)&~15ULL)+SEMANTIC_ACTION_PADDING;
 const uint64_t end=stub+sizeof semantic_action_sites/sizeof *semantic_action_sites*SEMANTIC_ACTION_STUB_BYTES;
 const uint64_t data=(m->file_size+0x3fff)&~0x3fffULL;
 if(((uintptr_t)m->base&0x3fff)||data+0x4000!=m->size||end>m->ro_off||end>((m->text_end+0x3fff)&~0x3fffULL))return 0;
 /* All guards and scratch space are checked before any byte is changed. */
 for(unsigned i=0;i<sizeof semantic_action_sites/sizeof *semantic_action_sites;i++){
  const SemanticActionSite *s=&semantic_action_sites[i];uint32_t word;
  if((uint64_t)s->offset+4>m->text_end)return 0;memcpy(&word,m->base+s->offset,4);if(word!=s->original)return 0;
  int64_t distance=(int64_t)(stub+i*SEMANTIC_ACTION_STUB_BYTES)-(int64_t)s->offset;
  if(distance<=-(1LL<<27)||distance>=(1LL<<27))return 0;
 }
 for(uint64_t i=stub;i<end;i++)if(m->base[i])return 0;
 for(unsigned i=0;i<sizeof semantic_action_sites/sizeof *semantic_action_sites;i++){
  uint64_t at=stub+i*SEMANTIC_ACTION_STUB_BYTES;
  int64_t pages=(int64_t)(data>>12)-(int64_t)((at+4)>>12);if(pages<-(1LL<<20)||pages>=(1LL<<20))return 0;
 }
 for(unsigned i=0;i<sizeof semantic_action_sites/sizeof *semantic_action_sites;i++){
  const SemanticActionSite *s=&semantic_action_sites[i];uint64_t at=stub+i*SEMANTIC_ACTION_STUB_BYTES;
  uint32_t pages=(uint32_t)((int64_t)(data>>12)-(int64_t)((at+4)>>12));unsigned shift=s->source*4;
  const uint32_t code[8]={0xa9bf47f0u, /* stp x16,x17,[sp,#-16]! */
   0x90000010u|((pages&3)<<29)|(((pages>>2)&0x7ffff)<<5), /* adrp x16,data */
   0x91000210u|(SEMANTIC_ACTION_DATA<<10), /* add x16,x16,#64 */
   0x88dffe11u, /* ldar w17,[x16]: one coherent mapping */
   0x53000230u|(shift<<16)|((shift+3)<<10), /* ubfx w16,w17,#shift,#4 */
   s->set?(0x2a1003e0u|s->reg):(0x2a100000u|(s->reg<<5)|s->reg),
   0xa8c147f0u, /* ldp x16,x17,[sp],#16 */semantic_action_branch(at+28,s->offset+4)};
  memcpy(m->base+at,code,sizeof code);uint32_t branch=semantic_action_branch(s->offset,at);memcpy(m->base+s->offset,&branch,4);
 }
 runtime_action_bindings_attach(m->base+data+SEMANTIC_ACTION_DATA);return 1;
}
