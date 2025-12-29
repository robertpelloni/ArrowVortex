#include <Core/Version.h>
#include <System/File.h>

namespace Vortex {
namespace Version {

static String g_version;

String Get() {
    if (g_version.empty()) {
        bool success;
        Vector<String> lines = File::getLines("VERSION.md", &success);
        if (success && !lines.empty()) {
            g_version = lines[0];
        } else {
            g_version = "0.0.0-dev";
        }
    }
    return g_version;
}

String GetBuildDate() {
    return __DATE__;
}

}
}
