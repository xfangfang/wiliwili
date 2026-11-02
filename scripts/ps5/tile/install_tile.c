/* wiliwili for the PlayStation 5
 *
 * Adds a wiliwili entry to the home screen of a jailbroken console. The entry is
 * a deeplink to the web app page that the websrv payload serves from the wiliwili
 * directory (index.html, see scripts/ps5/tile), which in turn starts the payload.
 *
 * The installation follows the approach of the ps5-payload-dev websrv and
 * launchpad payloads: the title parameters are written to /user/app and the
 * installer of the system is asked to register the directory, the entry appears
 * on the home screen next to the games.
 *
 * Usage: install_tile.elf [URL] [--uninstall]
 *   URL          base URL of the websrv payload, http://127.0.0.1:8080 by default
 *   --uninstall  remove a previously installed entry
 */

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <ps5/kernel.h>


#ifndef TITLE_ID
#define TITLE_ID "BREW99099"
#endif

// the directory the wiliwili payload is deployed to (scripts/ps5/deploy.sh), the
// deeplink below points at the web app page the websrv payload serves from it
#define APP_DIR "/data/homebrew/wiliwili"

// base URL of the websrv payload, replaced at runtime when an URL is passed
#define WEBSRV_URL "http://127.0.0.1:8080"

// the credentials the websrv payload picks before it installs its own entry,
// needed to write into /user/app and to talk to the installer of the system
#define AUTHID 0x4801000000000013L


// add a file to the payload, the size acts as the end marker, a zero byte
// terminates the data so that it can be used as a string
#define INCASSET(name, file)			\
  __asm__(".section .rodata\n"			\
	  ".global " #name "\n"			\
	  ".global " #name "_end\n"		\
	  ".global " #name "_size\n"		\
	  ".align 16\n"				\
	  #name ":\n"				\
	  ".incbin \"" file "\"\n"		\
	  #name "_end:\n"			\
	  ".byte 0\n"				\
	  ".align 8\n"				\
	  #name "_size:\n"			\
	  ".quad " #name "_end - " #name "\n"	\
	  ".previous\n");			\
  extern const uint8_t name[];			\
  extern const size_t name##_size;

INCASSET(param, "param.json");
INCASSET(icon0, "../sce_sys/icon0.png");


int sceAppInstUtilInitialize(void);
int sceAppInstUtilAppUnInstall(const char*);
int sceAppInstUtilAppInstallAll(void*);


static int
write_file(const char* path, const void* data, size_t size) {
  FILE* f;

  if(!(f=fopen(path, "w"))) {
    perror(path);
    return -1;
  }

  if(size && fwrite(data, size, 1, f) != 1) {
    perror(path);
    fclose(f);
    return -1;
  }

  return fclose(f);
}


// param.json as it was compiled in, with the base URL of the websrv payload
// replaced when the caller passed one
static int
write_param(const char* path, const char* url) {
  char buf[4096];
  const char* prefix = strstr((const char*)param, WEBSRV_URL);
  int len;

  if(!url || !prefix) {
    return write_file(path, param, param_size);
  }

  len = snprintf(buf, sizeof(buf), "%.*s%s%s",
		 (int)(prefix - (const char*)param), (const char*)param,
		 url, prefix + strlen(WEBSRV_URL));
  if(len < 0 || (size_t)len >= sizeof(buf)) {
    fprintf(stderr, "websrv URL is too long: %s\n", url);
    return -1;
  }

  return write_file(path, buf, len);
}


// sceAppInstUtilAppInstallTitleDir is not exported by the library of the SDK,
// the websrv and launchpad payloads resolve it by its NID as well
static int
install_title_dir(void) {
  int (*sceAppInstUtilAppInstallTitleDir)(const char*, const char*, void*) = 0;
  const char* nid = "Wudg3Xe3heE";
  uint32_t handle;

  if(!kernel_dynlib_handle(-1, "libSceAppInstUtil.sprx", &handle)) {
    sceAppInstUtilAppInstallTitleDir = (void*)kernel_dynlib_resolve(-1, handle, nid);
  }

  if(sceAppInstUtilAppInstallTitleDir) {
    return sceAppInstUtilAppInstallTitleDir(TITLE_ID, "/user/app/", 0);
  }

  return sceAppInstUtilAppInstallAll(0);
}


int
main(int argc, char* argv[]) {
  const char* url = 0;
  int uninstall = 0;
  int err;

  for(int i=1; i<argc; i++) {
    if(!strcmp(argv[i], "--uninstall")) {
      uninstall = 1;
    } else if(!strncmp(argv[i], "http://", 7) || !strncmp(argv[i], "https://", 8)) {
      url = argv[i];
    } else {
      fprintf(stderr, "usage: %s [URL] [--uninstall]\n", argv[0]);
      return 1;
    }
  }

  kernel_set_ucred_authid(-1, AUTHID);

  if((err=sceAppInstUtilInitialize())) {
    printf("sceAppInstUtilInitialize: 0x%08X\n", err);
    return 1;
  }

  if((err=sceAppInstUtilAppUnInstall(TITLE_ID))) {
    // the entry is not installed yet, that is not an error
    printf("sceAppInstUtilAppUnInstall(" TITLE_ID "): 0x%08X\n", err);
  }

  if(uninstall) {
    printf(TITLE_ID " removed\n");
    return 0;
  }

  if(mkdir("/user/app/" TITLE_ID, 0755) && errno != EEXIST) {
    perror("/user/app/" TITLE_ID);
    return 1;
  }
  if(mkdir("/user/app/" TITLE_ID "/sce_sys", 0755) && errno != EEXIST) {
    perror("/user/app/" TITLE_ID "/sce_sys");
    return 1;
  }

  if(write_param("/user/app/" TITLE_ID "/sce_sys/param.json", url)) {
    return 1;
  }
  if(write_file("/user/app/" TITLE_ID "/sce_sys/icon0.png", icon0, icon0_size)) {
    return 1;
  }

  if((err=install_title_dir())) {
    printf("sceAppInstUtilAppInstallTitleDir: 0x%08X\n", err);
    return 1;
  }

  printf("wiliwili added to the home screen, open it while websrv is running\n");
  return 0;
}
