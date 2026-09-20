#pragma once

// ============================================================
// Installer - download + extract + optional Aurora reload
// ------------------------------------------------------------
// Steps for a single package:
//   1. Download dataurl -> Hdd:\Anaconda\cache\package.7z
//   2. Create Hdd:\<pkg.path>\
//   3. Extract the .7z into that folder
//   4. Delete the temporary archive
//   5. If pkg.reload, call Platform::ReloadAurora()
//
// Returns true on success. All errors are logged and reported
// through the Ui::Message dialogs.
// ============================================================

#include "Anaconda.h"

namespace Installer {

    // Full install pipeline for one package.
    bool Install(const Package& pkg);

    // Install with a caller-supplied progress callback.
    // progress(bytesDone, bytesTotal) is called during download.
    bool Install(const Package& pkg,
                 void (*progress)(size_t, size_t));

} // namespace Installer
