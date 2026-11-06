// condemned2 - Condemned 2: Bloodshot (Xbox 360) on the Nintendo Switch.
//
// The app shell follows condemned-nx (Condemned 1), which follows ds2-nx; everything tied to the game itself
// (archives, hooks, recompile hints) is Condemned 2's own.
//
// Where the game is, next to the NRO (sdmc:/switch/condemned2-nx/ on the console):
//   game/     the extracted disc: default.xex, default.archcfg, autoexec.cfg, Layer01.Arch02, Layer02.Arch02,
//             game/ (loose videos and materials)   (tools/stage_sd.py makes it from your ISO)
//
// One disc, no title update: the recompiled code is the retail default.xex (Title ID 534507EF, version 0.0.0.1,
// built 2008-02-17, sha256 554b2ae2...0310fd).

#pragma once

#include "c2_menu.h"
#include <rex/cvar.h>
#include <rex/filesystem.h>
#include <rex/logging.h>
#include <rex/rex_app.h>
#include <rex/runtime.h>
#include <rex/system.h>
#include <rex/ui/overlay/achievement_notification.h>

#include <cstdlib>
#include <filesystem>
#include <memory>
#include <string>
#include <system_error>

#include "c2_cpu.h"
#if C2_DEKO3D
#include "c2_gpu.h"
#endif

#if defined(__SWITCH__)
#include <rex/ui/switch_apm.h>
#endif

// Handheld GPU clock through Horizon's own performance profiles (not an overclock), as ds2-nx and ds3-nx do.
REXCVAR_DEFINE_INT32(c2_gpu_portatil_mhz, 460, "C2",
                     "GPU in handheld mode through Horizon's performance profiles: 460 or 384 MHz; 0 = leave it "
                     "(307.2 MHz stock)")
    .range(0, 460);

REXCVAR_DEFINE_STRING(c2_game_dir, "", "C2", "Extracted disc folder. Empty: game/ next to the executable.");

class C2App : public rex::ReXApp {
 public:
  using rex::ReXApp::ReXApp;

  static std::unique_ptr<rex::ui::WindowedApp> Create(rex::ui::WindowedAppContext& ctx) {
    return std::unique_ptr<C2App>(new C2App(ctx, "c2", PPCImageConfig));
  }

 protected:
#if C2_DEKO3D
  // No Xenos emulation plugin in the deko3d build: c2_gpu.cpp answers the game's Direct3D and reads its ring.
  void OnPreSetup(rex::RuntimeConfig& config) override {
    if (!config.graphics) config.graphics = c2::gpu::CreateGraphicsSystem();
  }
#endif

  // game_data_root is the folder with default.xex: game/ next to the NRO (or c2_game_dir).
  void OnConfigurePaths(rex::PathConfig& paths) override {
    if (!paths.game_data_root.empty()) return;
    std::error_code ec;
    if (const std::string flag = REXCVAR_GET(c2_game_dir); !flag.empty()) {
      paths.game_data_root = flag;
      return;
    }
    const auto folder = rex::filesystem::GetExecutableFolder();
    if (folder.empty()) return;
    for (const auto& candidate : {folder / "game", folder / "disc1", folder}) {
      if (std::filesystem::is_regular_file(candidate / "default.xex", ec)) {
        paths.game_data_root = candidate;
        return;
      }
    }
  }

  void OnPostInitLogging() override {
    c2::cpu::InformarSistema();
    c2::menu::CargarOpciones();  // the options menu's saved choices, over c2.toml (before the cvars are final)
    DefaultFlag("input_vibracion", "true");
    DefaultFlag("input_vibracion_fuerza", "0.5");
#if !C2_DEKO3D
    DefaultFlag("gpu_plugin", "xenos");
#endif
  }

  void OnLoadXexImage(std::string& xex_image) override {
    std::error_code ec;
    const auto root = game_data_root();
    for (const char* f : {"Layer01.Arch02", "Layer02.Arch02", "default.archcfg"}) {
      if (!std::filesystem::exists(root / f, ec)) {
        Fail(std::string("condemned2: ") + f + " is missing from " + root.string() +
             " (extract the whole disc with tools/stage_sd.py)");
      }
    }
    REXLOG_INFO("condemned2: disc at {}; loading {}", root.string(), xex_image);
  }

  void OnPreLaunchModule() override {
    c2::cpu::Configurar();
    c2::menu::Instalar();
#if defined(__SWITCH__)
    if (const int32_t mhz = REXCVAR_GET(c2_gpu_portatil_mhz); mhz > 0) {
      RexSwitchApmPedirGpuMhz(mhz);
      RexSwitchApmAplicar();
    }
#endif
  }

  // No achievement toast: on the Switch it moves presentation to the UI thread (ds2-nx, ds3-nx).
  std::unique_ptr<rex::ui::AchievementNotificationDialog> CreateAchievementNotificationDialog() override {
    return nullptr;
  }

 private:
  static void DefaultFlag(const char* name, const char* value) {
    if (!rex::cvar::GetFlagInfo(name) || rex::cvar::GetFlagSource(name) != rex::cvar::Source::kDefault) return;
    rex::cvar::SetFlagByName(name, value);
  }

  [[noreturn]] static void Fail(const std::string& msg) {
    REXLOG_ERROR("{}", msg);
    rex::ShowSimpleMessageBox(rex::SimpleMessageBoxType::Error, msg);
    rex::FlushLogging();
    std::_Exit(1);
  }
};
