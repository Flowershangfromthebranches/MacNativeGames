/* Execute the production operation veneers on arm64, with synthetic guarded
   words only. No game image, player profile, controller or GPU is loaded. */
#define main original_control_probe_main
#include "test_control_input.c"
#undef main
#include "../games/totk/runtime/semantic_action_patch.h"
#include <sys/mman.h>
#include <libkern/OSCacheControl.h>
void probe_action_word(void *entry,uint32_t seed,uint64_t out[5]);
int main(void){
 RuntimeActionBindings b;action_bindings_default(&b);
 if(!action_version_supported()){assert(!runtime_action_bindings_set(&b));assert(!runtime_action_bindings_available());puts("PASS: unsupported game version refuses player action changes");return 0;}
 Module m={.text_end=0x1cd0100,.ro_off=0x1cd1000,.file_size=0x1cd8100,.size=0x1ce0000};
 m.base=mmap(NULL,m.size,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANON,-1,0);assert(m.base!=MAP_FAILED);
 for(unsigned i=0;i<6;i++){memcpy(m.base+semantic_action_sites[i].offset,&semantic_action_sites[i].original,4);uint32_t ret=0xd65f03c0;memcpy(m.base+semantic_action_sites[i].offset+4,&ret,4);}
 // A mismatch must leave all six words and scratch padding intact.
 uint32_t wrong=0;memcpy(m.base+semantic_action_sites[5].offset,&wrong,4);assert(!semantic_action_install(&m));
 for(unsigned i=0;i<5;i++){uint32_t word;memcpy(&word,m.base+semantic_action_sites[i].offset,4);assert(word==semantic_action_sites[i].original);}
 memcpy(m.base+semantic_action_sites[5].offset,&semantic_action_sites[5].original,4);
 assert(semantic_action_install(&m)&&runtime_action_bindings_available());
 assert(mprotect(m.base,(m.text_end+0x3fff)&~0x3fffULL,PROT_READ|PROT_EXEC)==0);sys_icache_invalidate(m.base,(m.text_end+0x3fff)&~0x3fffULL);
 // All allowed mappings execute through sealed RX code; Apply changes only RW data.
 static const unsigned choices[]={0,ACTION_INTERACT,ACTION_JUMP,ACTION_ATTACK};
 for(unsigned a=0;a<4;a++)for(unsigned x=0;x<4;x++)for(unsigned y=0;y<4;y++){
  b=(RuntimeActionBindings){{choices[a],choices[x],choices[y]}};assert(runtime_action_bindings_set(&b));
  for(unsigned i=0;i<6;i++){const SemanticActionSite *s=&semantic_action_sites[i];uint64_t out[5];probe_action_word(m.base+s->offset,0x20000,out);
   uint32_t expected=s->set?b.target[s->source]:(0x20000|b.target[s->source]);assert(out[s->reg-21]==expected);assert(out[22-s->reg]==0x20000);
   assert(out[2]==0x1234&&out[3]==0x5678&&out[4]==0x60000000);}
 }
 b=(RuntimeActionBindings){{ACTION_JUMP,ACTION_ATTACK,ACTION_INTERACT}};assert(runtime_action_bindings_set(&b));
 initialize();uint32_t id=0;NpadState state;static const unsigned keysToPress[]={38,40,34,32};static const unsigned raw[]={1,2,4,8};
 for(unsigned i=0;i<4;i++){runtime_input_key_synthetic(keysToPress[i],1);full_states(&state,1,&id);assert(state.buttons==raw[i]);runtime_input_key_synthetic(keysToPress[i],0);}
 RuntimeActionBindings bad={{2,4,8}},got;assert(!runtime_action_bindings_set(&bad));runtime_action_bindings_get(&got);assert(action_bindings_pack(&got)==0x184);
 action_bindings_default(&b);assert(runtime_action_bindings_set(&b));uint64_t out[5];probe_action_word(m.base+semantic_action_sites[0].offset,0x20000,out);assert(out[1]==ACTION_INTERACT);
 runtime_action_bindings_attach(NULL);assert(!runtime_action_bindings_available());assert(!runtime_action_bindings_set(&b));
 munmap(m.base,m.size);puts("PASS: guarded live player actions, RX text, registers/flags, Apply/reset and unchanged raw menu A/B/X/Y");return 0;
}
