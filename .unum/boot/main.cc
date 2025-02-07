/*-------------------------------------------------------------------
| vi: set noet ts=4 sw=4 fenc=utf-8
| -------------------------------------------------------------------
| Copyright (c) 2025 Francis Henry Grolemund III and RealProven, LLC.
| SPDX-License-Identifier: LicenseRef-Unum-Commercial OR GPL-3.0-only
| -------------------------------------------------------------------*/

/*
 *  The purpose of bootstrapping is to ingest the C++ compiler environment
 *  from a platform-specific toolchain (ie. `make`) into the
 *  platform-independent `unum` toolchain to allow the latter take over all
 *  further build processing.  
 *
 *  This program figures out where it is running, encodes fundamental 
 *  configuration and builds the first limited `unum` kernel, called the 
 *  pre-kernel.  The objective is for it to be as minimal as possible
 *  without confusing the process or introducing needless waste if/when `make`
 *  is re-invoked on an existing repo, while providing enough to build the
 *  full unum kernel to take over future deployment responsibility.
 *
 *  ASSUMPTION: Built using the makefile in the root while the CWD is the root.
 */

#include <ctype.h>
#include <dirent.h>
#include <limits.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// ...test that this is a C++ compiler
#include <cstdarg>
#include <cstdio>
#include <cstdlib>


typedef enum {
	A_CXX = 0,
	A_LD,

	A_COUNT
} arg_e;

typedef enum {
	P_UNKNOWN = 0,
	P_MACOS,
} platform_e;

typedef const char **cstrarr_t;


static cstrarr_t arr_add( cstrarr_t arr, const char *text );
static void build_pre_k( void );
static void config_basis( void );
static void detect_path_style( void );
static void detect_platform( void );
static struct stat file_info( const char *path );
static char *find_in_path( const char *cmd );
static bool last_header_mod( const char *dir_path, time_t *last_mod );
static const char *parse_option( const char *optName, const char *from );
static void parse_cmd_line( int argc, char *argv[] );
static void printf_config( const char *fmt, ... );
static const char *resolve_cmd( const char *cmd );
static void read_manifest( cstrarr_t *inc_dirs, cstrarr_t *src_files,
                           time_t *last_mod );
static char *rstrcat( char *buf, const char *text );
static int run_cc( const char *bin_file, cstrarr_t pp_defs, cstrarr_t inc_dirs,
                   cstrarr_t src_files );
static int run_cc_with_source( const char *source );
static bool s_ends_with( const char *text, const char *suffix );
static cstrarr_t to_arr( const char *text, ... /* NULL */ );
static const char *to_repo( const char *path, bool from_basis = true );
static const char *trim_ws( char *text );
static void uabort( const char *fmt, ... );
static void write_config( void );


#define BASIS_DIR          to_repo(NULL)
#define DEPLOYED_DIR       to_repo("deployed")
#define TEMP_DIR           to_repo("deployed/temp")
#define BUILD_DIR          to_repo("deployed/build")
#define BUILD_INCLUDE_DIR  to_repo("deployed/build/include")
#define BIN_DIR            to_repo("deployed/bin")
#define UCONFIG_FILE       to_repo("deployed/build/include/u_config.h")
#define UKERN_FILE         to_repo("deployed/bin/unum")
#define MANIFEST_FILE      to_repo("config/manifest.umy")
#define DEBUG_ENV          "UBOOT_DEBUG"
#define MAN_SEC_CORE       "core:"
#define MAN_SEC_BUILD      "build:"
#define MAN_SEC_INC        "include:"
#define is_path_sep(c)      (c == '/' || c == '\\')
#define is_file(p)         (file_info((p)).st_mode & S_IFREG)
#define is_dir(p)          (file_info((p)).st_mode & S_IFDIR)
#define path_sep_s         ((char [2]) { path_sep, '\0' })
#define CFG_SIZE           32768
#define IS_UNIX            (platform == P_MACOS)
#define assert(e)          if (!(e)) uabort("assert failed, line %d", __LINE__)


static struct { arg_e arg;
                const char *name; 
                const char *value; } bargs[] = {{ A_CXX, "cpp", NULL },
	                                            { A_LD,  "link", NULL }
                                               };
static char        root_dir[PATH_MAX];
static char        config[CFG_SIZE];
static char        *cfg_offset               = config;
static char        path_sep                  = '\0';
static platform_e  platform                  = P_UNKNOWN;
static FILE        *uberr                    = NULL;


int main( int argc, char *argv[] ) {
	// - don't spam stderr with system()
	uberr = fdopen(dup(fileno(stderr)), "w");
	if (!getenv(DEBUG_ENV)) {
		fclose(stderr);
	}
	
	// - order is important here
	detect_path_style();
	parse_cmd_line(argc, argv);
	config_basis();
	detect_platform();
	write_config();
	build_pre_k();

	fclose(uberr);
	return 0;
}


static void detect_path_style( void ) {
	if (!getcwd(root_dir, PATH_MAX)) {  // - must be the root of the repo!
		uabort("failed to detect CWD.");
	}
	
	for (char *rp = root_dir; *rp ; rp++) {
		if (is_path_sep(*rp)) {
			path_sep = *rp;
			break;
		}
	}
	
	if (!path_sep) {
		uabort("failed to detect path sep");
	}

	if (!is_dir(to_repo(NULL))) {
		uabort("no basis, invalid unum repo");
	}
}


static void uabort( const char *fmt, ... ) {
	va_list ap;
	char    buf[2048];

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	fprintf(uberr ? uberr : stdout, "uboot error: %s\n", buf);
	abort();
}


static void parse_cmd_line( int argc, char *argv[] ) {
	const char  *opt = NULL;
	int         i;

	while (--argc) {
		const char *item = *++argv;
		int is_sup = 0;
		for (i = 0; i < sizeof(bargs)/sizeof(bargs[0]); i++) {
			assert(bargs[i].arg == i);
			if ((opt = parse_option(bargs[i].name, item))) {
				opt = (opt && opt[0]) ? opt : NULL;
				opt = (i == A_CXX || i == A_LD) ?
				       resolve_cmd(opt) : opt;
				bargs[i].value = opt;
				is_sup = 1;
				break;
			}	
		}

		if (!is_sup) {
			uabort("unsuported command-line parameter '%s'", item);
		}
	}

	if (!bargs[A_CXX].value || !bargs[A_LD].value) {
		uabort("missing one or more required tool parameters.");
	}
}


static const char *parse_option( const char *opt_name, const char *from ) {
	char       prefix[64] = "\0";
	const char *p         = prefix;

	snprintf(prefix, sizeof(prefix), "--%s=", opt_name);

	while (*p) {
		if (*p++ != *from++) {
			return NULL;
		}
	}

	return from;
}


static const char *resolve_cmd( const char *cmd ) {
	static char tmp[PATH_MAX];
	const char *rc            = NULL;

	if (!cmd || !cmd[0]) {
		return NULL;
	}

	const int has_dir = strstr(cmd, path_sep_s) != NULL;
	const int is_rel  = cmd[0] == '.';

	if (is_rel) {
		getcwd(tmp, PATH_MAX);
		strcat(tmp, path_sep_s);
		strcat(tmp, cmd);
		rc = tmp;

	} else if (has_dir) {
		rc = cmd;

	} else {
		rc = find_in_path(cmd);	
	}

	if (!rc || !is_file(rc) || (rc = strdup(rc)) == NULL) {
		uabort("unresolvable command path '%s'", cmd);
	}

	return rc;
}


static char *find_in_path( const char *cmd ) {
	const char   *path         = getenv("PATH");
	static char  buf[PATH_MAX];
	char         *ppos         = buf;
	char         c;

	while ((c = *path++)) {
		if (c == ':' || c == ';') {
			*ppos++ = path_sep;
			strcpy(ppos, cmd);

			if (is_file(buf)) {
				return buf;
			}

			ppos = buf;
		} else {
			*ppos++ = c;
		}
	}

	return NULL;
}


static struct stat file_info( const char *path ) {
	struct stat sinfo;

	if (path && stat(path, &sinfo) == 0) {
		return sinfo;
	}

	memset(&sinfo, 0, sizeof(sinfo));
	return sinfo;
}


static void config_basis( void ) {
	const char *build_dirs[] = { DEPLOYED_DIR, TEMP_DIR, BUILD_DIR,
								 BUILD_INCLUDE_DIR, BIN_DIR, NULL };
	                              
	for (const char **bd = build_dirs; *bd; bd++) {
		if (!is_dir(*bd) && mkdir(*bd, S_IRWXU) != 0) {
			uabort("failed to create build directory '%s'", bd);
		}
	}
}


static void detect_platform( void ) {
	if (run_cc_with_source(
	           "#include <Carbon/Carbon.h>\n"	
	           "#include <cstdio>\n\n"
	           "int main(int argc, char **argv) {\n"
	           "  printf(\"hello unum\");\n"
	           "}\n"
	          ) == 0) {
		platform = P_MACOS;
		return;
	}

	uabort("unsupported platform type");
}


static int run_cc_with_source( const char *source ) {
	const char *tmp_env[]         = { "TMPDIR", "TMP", "TEMP", "TEMPDIR", 
	                                  NULL };
	const char **tp               = tmp_env;
	int        rc;
	char       src_name[PATH_MAX];
	char       bin_name[PATH_MAX];
	FILE       *src_file;

	// ...avoids the scary warning from macos for using tmpnam
	do {
		const char *e_val = getenv(*tp);

		if (e_val) {
			snprintf(src_name, PATH_MAX, "%s%cunum-boot.cc", e_val, path_sep);
			break;	
		}

		if (!*++tp) {
			uabort("failed to find temp directory!.");
		}
	} while (*tp);

	snprintf(bin_name, PATH_MAX, "%s.out", src_name);

	src_file = fopen(src_name, "w");
	if (!src_file || !fwrite(source, strlen(source), 1, src_file)) {
		uabort("failed to create temp file.");
	}
	fclose(src_file);

	rc = run_cc(bin_name, NULL, NULL, to_arr(src_name, NULL));

	unlink(src_name);
	unlink(bin_name);

	return rc;
}


static cstrarr_t to_arr( const char *text, ... /* NULL */ ) {
	cstrarr_t   ret   = NULL;
	va_list     ap;
	const char  *item = text;
	
	va_start(ap, text);
	while (item) {
		ret  = arr_add(ret, item);
		item = va_arg(ap, const char *);
	}
	va_end(ap);
	
	return ret;
}


static cstrarr_t arr_add( cstrarr_t arr, const char *text ) {
	cstrarr_t ret = NULL;
	int       len = 0;
	
	if (!text) {
		return arr;
	}
	
	for (cstrarr_t cur = arr; cur && *cur; cur++) {
		len++;
	}
	
	ret = (cstrarr_t) realloc(arr, sizeof(char *) * (len + 2));
	if (!ret) {
		uabort("out of memory");
	}
	
	ret[len] = strdup(text);
	if (!ret[len]) {
		uabort("out of memory");
	}
	
	ret[len+1] = NULL;
	return ret;
}


// - arrays must be terminated with NULL
static int run_cc( const char *bin_file, cstrarr_t pp_defs, cstrarr_t inc_dirs,
                   cstrarr_t src_files ) {
	char *cmd = NULL;
	
	assert(src_files);
	
	cmd = rstrcat(cmd, bargs[A_CXX].value);

	for (; inc_dirs && *inc_dirs && **inc_dirs; inc_dirs++) {
		cmd = rstrcat(cmd, " -I");
		cmd = rstrcat(cmd, *inc_dirs);
	}

	for (; pp_defs && *pp_defs && **pp_defs; pp_defs++) {
		cmd = rstrcat(cmd, " -D");
		cmd = rstrcat(cmd, *pp_defs);
	}

	cmd = rstrcat(cmd, " -o ");
	cmd = rstrcat(cmd, bin_file);

	for (; *src_files && *src_files; src_files++) {
		cmd = rstrcat(cmd, " ");
		cmd = rstrcat(cmd, *src_files);
	}

	return system(cmd);
}


static char *rstrcat( char *buf, const char *text ) {
	const size_t len_cur = buf ? strlen(buf) : 0;
	const size_t len_txt = strlen(text);

	if (!len_txt) {
		return buf;
	}

	buf = (char *) realloc(buf, len_cur + len_txt + 1);
	if (!buf) {
		uabort("out of memory");
	}

	strcpy(&buf[len_cur], text);
	return buf;
}


static const char *to_repo( const char *path, bool from_basis ) {
	static char buf[PATH_MAX];
	char *bp                   = buf;
	
	for (const char *rp = root_dir; *rp; rp++, bp++) {
		*bp = *rp;
	}
	
	*bp++ = path_sep;
		
	if (from_basis) {
		for (const char *up = ".unum"; *up; up++, bp++) {
			*bp = *up;
		}
		*bp++ = path_sep;
	}
	
	for (const char *pp = path; pp && *pp; pp++, bp++) {
		*bp = is_path_sep(*pp) ? path_sep : *pp;
	}
	
	*bp = '\0';
	
	bp = strdup(buf);
	if (!bp) {
		uabort("out of memory");
	}
	
	return bp;
}


static void write_config( void ) {
	FILE        *fp;
	char        buf[CFG_SIZE];
	const char  *cfg_file;

	memset(config, 0, sizeof(config));
	printf_config("#ifndef UNUM_CONFIG_H");	
	printf_config("#define UNUM_CONFIG_H");	
	printf_config("");
	printf_config("/*");
	printf_config(" *  This file is auto-generated.");
	printf_config(" */");
	printf_config("");


	printf_config("#define UNUM_OS_UNIX         %d", IS_UNIX ? 1 : 0);	
	printf_config("%s#define UNUM_OS_MACOS        %d",
	                platform == P_MACOS ? "" : "// ", 
	                platform == P_MACOS ? 1 : 0);
	printf_config("");


	printf_config("#define UNUM_PATH_SEP        '%c'", path_sep);
	printf_config("#define UNUM_PATH_SEP_S      \"%c\"", path_sep);
	printf_config("");


	// - defining these here guarantee that if the code is moved to a different
	//   root, the configuration will look different and trigger rebuilds
	printf_config("#define UNUM_DIR_ROOT        \"%s\"", root_dir);
	printf_config("#define UNUM_DIR_BASIS       \"%s\"", BASIS_DIR);
	printf_config("#define UNUM_BASIS_DEPLOY    \"%s\"", DEPLOYED_DIR);
	printf_config("#define UNUM_BASIS_BUILD     \"%s\"", BUILD_DIR);
	printf_config("#define UNUM_BASIS_INCLUDE   \"%s\"", BUILD_INCLUDE_DIR);
	printf_config("#define UNUM_BASIS_BIN       \"%s\"", BIN_DIR);
	printf_config("#define UNUM_MANIFEST        \"%s\"", MANIFEST_FILE);
	printf_config("");
	printf_config("#define UNUM_RUNTIME_BIN     \"%s\"", UKERN_FILE);
	printf_config("");


	printf_config("#define UNUM_TOOL_CXX        \"%s\"", bargs[A_CXX].value);
	printf_config("#define UNUM_TOOL_LD         \"%s\"", bargs[A_LD].value);
	printf_config("");

	printf_config("#endif /* UNUM_CONFIG_H */");

	cfg_file = UCONFIG_FILE;
	fp       = fopen(cfg_file, "r");
	if (fp) {
		size_t rc = fread(buf, 1, CFG_SIZE, fp);
		int err   = ferror(fp);	
		fclose(fp);
		if (rc > 0 && !err && !strcmp(buf, config)) {
			return;
		}
	}

	fp = fopen(cfg_file, "w");
	if (!fp || fwrite(config, 1, strlen(config), fp) == 0) {
		uabort("failed to generate config '%s'", cfg_file);
	}
	fclose(fp);
}


static void printf_config( const char *fmt, ... ) {
	va_list ap;
	va_start(ap, fmt);
	cfg_offset += vsnprintf(cfg_offset, sizeof(config) - (cfg_offset - config),
	                        fmt, ap);
	va_end(ap);
	*cfg_offset++ = '\n';
}


static void build_pre_k( void ) {
	time_t      last_mod             = 0;
	cstrarr_t   inc_dirs, src_files;
	char        bin_file[PATH_MAX];
	struct stat s;
	int         rc;
	
	read_manifest(&inc_dirs, &src_files, &last_mod);
	
	strcpy(bin_file, UKERN_FILE);
	
	if ((s = file_info(bin_file)).st_mode & S_IFREG && s.st_mtime >= last_mod) {
		return;
	}
	
	rc = run_cc(bin_file, to_arr("UNUM_BOOTSTRAP", NULL), inc_dirs, src_files);
	if (rc != 0) {
		uabort("failed to build pre-k, rc=%d", rc);
	}
	
	printf("uboot: bootstrapping prepared\n");
}

/*
 *  <sample-manifest>
 *
 *  core:
 *    - .unum/src/deploy/d_deploy.cc
 *    - .unum/src/main.cc
 *
 *  kernel:
 *
 *  build:
 *    include:
 *      - .unum/build/include
 *      - .unum/src
 *
 */
static void read_manifest( cstrarr_t *inc_dirs, cstrarr_t *src_files,
                           time_t *last_mod ) {
	FILE        *fp;
	char        buf[8192];
	char        *bp;
	int         is_core = 0, is_build = 0, is_inc = 0;
	int         line = 0;
	struct stat s;
	
	*inc_dirs  = NULL;
	*src_files = NULL;
	
	// - the pre-kernel only requires 'core' and 'include' content
	fp = fopen(MANIFEST_FILE, "r");
	while (fp && !feof(fp) && fgets(buf, sizeof(buf), fp)) {
		line++;
		
		if (!strncmp(buf, MAN_SEC_CORE, strlen(MAN_SEC_CORE))) {
			is_core  = 1;
			is_build = is_inc = 0;
			continue;
				
		} else if (!strncmp(buf, MAN_SEC_BUILD, strlen(MAN_SEC_BUILD))) {
			is_build = 1;
			is_core  = is_inc = 0;
			continue;
			
		} else if ((is_core || is_build) && !isspace(*buf)) {
			is_build = is_core = is_inc = 0;
			continue;
			
		} else if (!is_core && !is_build) {
			continue;
		}
		
		for (bp = buf; *bp && isspace(*bp); bp++) {}
		
		if (is_core) {
			if (*bp == '-' && isspace(*(bp+1)) && !isspace(*bp+2)) {
				bp += 2;
				
				if (!trim_ws(bp) || !*bp ||
				    !((s = file_info(bp)).st_mode & S_IFREG)) {
					uabort("invalid manifest file %s, line %d", bp, line);
				}
				
				if (s.st_mtime > *last_mod) {
					*last_mod = s.st_mtime;
				}
				
				*src_files = arr_add(*src_files, bp);
			}

		} else if (is_build) {
			if (is_inc) {
				if (*bp == '-' && isspace(*(bp + 1)) && !isspace(*bp+2)) {
					bp += 2;
				
					if (!trim_ws(bp) || !*bp ||
					    !last_header_mod(bp, last_mod)) {
						uabort("invalid manifest include %s, line %d", bp, line);
					}
					
					*inc_dirs = arr_add(*inc_dirs, bp);
					
				} else if (*bp && !isspace(*bp)) {
					is_inc = 0;
				}
			
			} else {
				if (!strncmp(bp, MAN_SEC_INC, strlen(MAN_SEC_INC))) {
					is_inc = 1;
				}
			}
		}
	}
	
	if (!fp || ferror(fp)) {
		uabort("failed to read manifest");
	}

	fclose(fp);
}


static const char *trim_ws( char *text ) {
	char *tp = text;
	
	for (tp = text; tp && *tp; tp++) {}
	for (tp--; tp && tp > text && isspace(*tp); tp--) {
		*tp = '\0';
	}
	
	return text;
}


static bool last_header_mod( const char *dir_path, time_t *last_mod ) {
	DIR *dirp      = NULL;
	bool ret       = true;
	char *file;
	struct stat s;
	
	dirp = opendir(dir_path);
	if (!dirp) {
		return false;
	}
	
	while (struct dirent *ditem = readdir(dirp)) {
		file = rstrcat(NULL, dir_path);
		file = rstrcat(file, path_sep_s);
		file = rstrcat(file, ditem->d_name);
		
		if (ditem->d_type == DT_DIR) {
			if ((strcmp(ditem->d_name, ".") && strcmp(ditem->d_name, "..")) &&
			    !last_header_mod(file, last_mod)) {
				ret = false;
				break;
			}
			
		} else {
			if ((s = file_info(file)).st_mode & S_IFREG &&
			    s_ends_with(file, ".h") && s.st_mtime > *last_mod) {
			    *last_mod = s.st_mtime;
			}
		}
	}

	closedir(dirp);
	return ret;
}


static bool s_ends_with( const char *text, const char *suffix ) {
	bool ret                = false;
	const char *ctmp, *stmp;
	
	assert(suffix && suffix[0]);
	
	for (const char *cur = text; *cur && !ret; cur++) {
		if (*cur != *suffix) {
			continue;
		}
		
		for (ctmp = cur, stmp = suffix;;ctmp++, stmp++) {
			if (!*ctmp || !*stmp) {
				if (!*ctmp && !*stmp) {
					ret = true;
				}
				break;
			}
		}
	}
	
	return ret;
}
