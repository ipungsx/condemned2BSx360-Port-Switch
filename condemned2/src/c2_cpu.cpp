// condemned - the recompiled code's "interrupts off" regions and the first lines of the log.
// From ds2-nx's ds2_cpu.cpp, without its EA-engine hooks (Condemned is LithTech; its hooks will come from profiling).

#include "condemned2_pch.h"

#include <rex/cvar.h>
#include <rex/logging.h>

#include "c2_cpu.h"

#if defined(__SWITCH__)
#include <switch.h>
#endif

REXCVAR_DECLARE(bool, switch_atajos_depuracion);  // the SDK's ZL + ZR shortcuts

REXCVAR_DEFINE_BOOL(c2_msr_cerrojo_global, false, "C2",
                    "mtmsrd regions of the recompiled code as one process-wide mutex, as the codegen makes them. "
                    "false = per-thread state (the atomics are already compare-and-swap)");

namespace c2::cpu {

void Configurar() {
#if !C2_DEPURACION
  if (REXCVAR_GET(switch_atajos_depuracion)) {
    REXCVAR_SET(switch_atajos_depuracion, false);
    REXLOG_INFO("[cpu] switch_atajos_depuracion ignored: release build (FPS counter only)");
  }
#endif
  ppc_msr_cerrojo_global_() = REXCVAR_GET(c2_msr_cerrojo_global);
  REXLOG_INFO("[cpu] mtmsrd regions: {}", ppc_msr_cerrojo_global_() ? "global mutex (codegen)" : "per thread");
}

void InformarSistema() {
#if defined(__SWITCH__)
  const u32 hos = hosversionGet();
  const bool aplicacion = appletGetAppletType() == AppletType_Application;
  REXLOG_INFO("condemned2-nx {} (commit {}), firmware {}.{}.{}{}, {}, {}", C2_VERSION, C2_COMMIT, HOSVER_MAJOR(hos),
              HOSVER_MINOR(hos), HOSVER_MICRO(hos), hosversionIsAtmosphere() ? " (Atmosphere)" : "",
              aplicacion ? "title takeover" : "APPLET MODE",
              appletGetOperationMode() == AppletOperationMode_Handheld ? "handheld" : "docked");
  if (!aplicacion) {
    REXLOG_WARN("condemned2-nx: started as an applet (from the album?): not enough memory. Start any game while "
                "holding R and open the Homebrew Menu from there");
  }
#else
  REXLOG_INFO("condemned2-nx {} (commit {})", C2_VERSION, C2_COMMIT);
#endif
}

}  // namespace c2::cpu

// The PCH declares the check hook of patch_local_regs.py; that pass is off in this port for now.
extern "C" void c2_regcheck_fail(const char*, const char*, int, const char*, const uint64_t*, const PPCContext*) {}
