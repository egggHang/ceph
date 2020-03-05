#ifndef OPENSSL_ACCEL_ENGINE_H
#define OPENSSL_ACCEL_ENGINE_H

#include "common/debug.h"
#include "global/global_context.h"

#include <openssl/engine.h>
// ----------------------------------------------------------------------------- 
#define dout_context g_ceph_context 
#define dout_subsys ceph_subsys_crypto 
#undef dout_prefix 
#define dout_prefix _prefix(_dout)
 
static ostream& 
_prefix(std::ostream* _dout) { 
  return *_dout << "OpensslAccelEngine: "; 
} 
// ----------------------------------------------------------------------------- 

class OpensslAccelEngine {
 private:
  explicit OpensslAccelEngine(string eid) {
    if (engine_type != "none") {
      ENGINE_load_dynamic();

      std::unique_ptr<ENGINE, std::function<void(ENGINE*)>> engine = {
          ENGINE_by_id(engine_type.c_str()), [](ENGINE* e) { ENGINE_free(e); }};

      if (engine.get()) {
        if (ENGINE_init(engine.get()) == 1) {
          engine.get_deleter() = [](ENGINE* e) {
            ENGINE_finish(e);
            ENGINE_free(e);
          };
          engine_ = std::move(engine);
        } else {
          derr << "failed to init engine: " << eid << dendl;
        }
      } else {
        derr << "failed to get engine: " << eid << dendl;
      }
    }
  }

  OpensslAccelEngine(const OpensslAccelEngine&) = delete;
  OpensslAccelEngine& operator=(const OpensslAccelEngine&) = delete;

 public:
  static OpensslAccelEngine& get_instance(string eid) {
    if (eid == "kae") {
      static OpensslAccelEngine kae(eid);
      return kae;
    } else {
      derr << "invalid engine: " << eid << dendl;
      static OpensslAccelEngine no_engine("none");
      return no_engine;
    }
  }

  ENGINE* get_engine() { return engine_.get(); }

 private:
  std::unique_ptr<ENGINE, std::function<void(ENGINE*)>> engine_;
};

#endif
