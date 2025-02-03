/*-------------------------------------------------------------------
| vi: set noet ts=4 sw=4 fenc=utf-8
| -------------------------------------------------------------------
| Copyright (c) 2025 Francis Henry Grolemund III and RealProven, LLC.
| SPDX-License-Identifier: LicenseRef-Unum-Commercial OR GPL-3.0-only
| -------------------------------------------------------------------*/

#include <cctype>
#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>

#include "d_deploy.h"

class deployment {
	public:
	
	deployment() {
		heap_allocs = nullptr;
		num_alloc   = 0;
		max_alloc   = 0;
	}
	
	bool deploy( char *error, size_t len ) {
		cstrarr_t inc_dirs, src_files;
		
		try {
			set_root();
			read_manifest(&inc_dirs, &src_files);
			run_cc(UNUM_RUNTIME_BIN, inc_dirs, src_files);
			
		} catch (uabort &err) {
			std::strncpy(error, err.msg, len);
			return false;
			
		}
		catch (...) {
			std::strncpy(error, "unexpected exception", len);
			return false;
		}
		
		return true;
	}


	~deployment() {
		for (int i = 0; i < num_alloc; i++) {
			::free(heap_allocs[i]);
		}
		::free(heap_allocs);
	}


	private:
	
	typedef const char **cstrarr_t;
			
	class uabort {
		public:
		char msg[512];
		uabort(const char *fmt, ...) {
			va_list ap;

			va_start(ap, fmt);
			vsnprintf(msg, sizeof(msg), fmt, ap);
			va_end(ap);
		}
	};
	
	
	void set_root( void ) {
		if (chdir(UNUM_DIR_ROOT) != 0) {
			throw uabort("failed to set root directory");
		}
	}
	
	
	const static char *MAN_SEC_CORE;
	const static char *MAN_SEC_KERNEL;
	const static char *MAN_SEC_BUILD;
	const static char *MAN_SEC_INC;
	
	
	void read_manifest( cstrarr_t *inc_dirs, cstrarr_t *src_files ) {
		FILE        *fp;
	
		*inc_dirs  = NULL;
		*src_files = NULL;
	
		try {
			fp = std::fopen(UNUM_MANIFEST, "r");
			if (!fp) {
				throw uabort("failed to read manifest");
			}
			
			// - the manifest is organized from lowest-to-highest abstraction
			//   in-order to satisfy link dependencies.
			read_manifest_from(fp, MAN_SEC_INC, inc_dirs, src_files);
			std::fseek(fp, 0L, SEEK_SET);
			read_manifest_from(fp, MAN_SEC_CORE, inc_dirs, src_files);
			std::fseek(fp, 0L, SEEK_SET);
			read_manifest_from(fp, MAN_SEC_KERNEL, inc_dirs, src_files);

		} catch (...) {
			if (fp) {
				std::fclose(fp);
			}
			throw;
		}
	}
	
	
	void read_manifest_from( FILE *fp, const char *section, cstrarr_t *inc_dirs,
	                         cstrarr_t *src_files ) {
		char        buf[8192];
		char        *bp;
		int         is_core = 0, is_kern = 0, is_build = 0, is_inc = 0;
		int 		do_core = 0, do_kern = 0, do_inc = 0;
		int         line = 0;
		struct stat s;
		
		do_core = !str2cmp(section, MAN_SEC_CORE);
		do_kern = !str2cmp(section, MAN_SEC_KERNEL);
		do_inc  = !str2cmp(section, MAN_SEC_INC);

		while (fp && !std::feof(fp) && std::fgets(buf, sizeof(buf), fp)) {
			line++;
		
			if (!str2cmp(buf, MAN_SEC_CORE)) {
				is_core  = 1;
				is_kern  = is_build = is_inc = 0;
				continue;

			}
			else if (!str2cmp(buf, MAN_SEC_KERNEL)) {
				is_kern  = 1;
				is_core  = is_build = is_inc = 0;
				continue;
								
			} else if (!str2cmp(buf, MAN_SEC_BUILD)) {
				is_build = 1;
				is_core  = is_kern = is_inc = 0;
				continue;
			
			} else if ((is_core || is_build) && !std::isspace(*buf)) {
				is_core = is_kern = is_build = is_inc = 0;
				continue;
			
			} else if (!is_core && !is_build && !is_kern) {
				continue;
			}
		
			for (bp = buf; *bp && std::isspace(*bp); bp++) {}
		
			if ((is_core && do_core) || (is_kern && do_kern)) {
				if (*bp == '-' && std::isspace(*(bp+1)) &&
				    !std::isspace(*bp+2)) {
					bp += 2;
				
					if (!trim_ws(bp) || !*bp ||
				        !((s = file_info(bp)).st_mode & S_IFREG)) {
						throw uabort("invalid manifest file %s, line %d", bp,
					                 line);
					}
				
					*src_files = arr_add(*src_files, bp);
				}

			} else if (is_build && do_inc) {
				if (is_inc) {
					if (*bp == '-' && std::isspace(*(bp + 1)) &&
					    !std::isspace(*bp+2)) {
						bp += 2;
				
						if (!trim_ws(bp) || !*bp) {
							throw uabort("invalid manifest include %s, line %d",
							             bp, line);
						}
					
						*inc_dirs = arr_add(*inc_dirs, bp);
					
					} else if (*bp && !std::isspace(*bp)) {
						is_inc = 0;
					}
			
				} else {
					if (!str2cmp(bp, MAN_SEC_INC)) {
						is_inc = 1;
					}
				}
			}
		}
	
		if (std::ferror(fp)) {
			throw uabort("failed to read manifest");
		}
	}
	
	
	// ...compare the prefix of s1 precisely to s2
	inline int str2cmp(const char *s1, const char *s2) {
		return strncmp(s1, s2, s2 ? strlen(s2) : 0);
	}

				
	struct stat file_info( const char *path ) {
		struct stat sinfo;

		if (path && stat(path, &sinfo) == 0) {
			return sinfo;
		}

		memset(&sinfo, 0, sizeof(sinfo));
		return sinfo;
	}
	
	
	const char *trim_ws( char *text ) {
		char *tp = text;
	
		for (tp = text; tp && *tp; tp++) {}
		for (tp--; tp && tp > text && std::isspace(*tp); tp--) {
			*tp = '\0';
		}
	
		return text;
	}
	

	cstrarr_t arr_add( cstrarr_t arr, const char *text ) {
		cstrarr_t ret = NULL;
		int       len = 0;
	
		if (!text) {
			return arr;
		}
	
		for (cstrarr_t cur = arr; cur && *cur; cur++) {
			len++;
		}
	
		ret        = (cstrarr_t) realloc(arr, sizeof(char *) * (len + 2));
		ret[len]   = strdup(text);
		ret[len+1] = NULL;
		return ret;
	}
	
		
	void run_cc( const char *bin_file, cstrarr_t inc_dirs,
	            cstrarr_t src_files ) {
		char *cmd = NULL;
	
		cmd = rstrcat(cmd, UNUM_TOOL_CXX);

		for (; inc_dirs && *inc_dirs && **inc_dirs; inc_dirs++) {
			cmd = rstrcat(cmd, " -I");
			cmd = rstrcat(cmd, *inc_dirs);
		}

		cmd = rstrcat(cmd, " -o ");
		cmd = rstrcat(cmd, bin_file);

		for (; *src_files && *src_files; src_files++) {
			cmd = rstrcat(cmd, " ");
			cmd = rstrcat(cmd, *src_files);
		}

		if (system(cmd) != 0) {
			throw uabort("failed to deploy kernel");
		}
	}
	
			
	char *rstrcat( char *buf, const char *text ) {
		const size_t len_cur = buf ? strlen(buf) : 0;
		const size_t len_txt = strlen(text);

		if (!len_txt) {
			return buf;
		}

		buf = (char *) realloc(buf, len_cur + len_txt + 1);
		if (!buf) {
			throw uabort("out of memory");
		}

		std::strcpy(&buf[len_cur], text);
		return buf;
	}


	// - despite porting uboot algo for v1, this must prevent leaks!
	void **heap_allocs;
	int  num_alloc;
	int  max_alloc;
	
	void *realloc(void *ptr, size_t len) {
		if (len == 0) {
			return nullptr;
		}
	
		int pos = -1;
		if (ptr) {
			for (int i = 0; i < num_alloc; i++) {
				if (heap_allocs[i] == ptr) {
					pos = i;
					break;
				}
			}
			
			if (pos == -1) {
				throw uabort("invalid pointer for realloc");
			}
						
		} else {
			if (num_alloc == max_alloc) {
				max_alloc += 10;
				heap_allocs = (void **)
						  std::realloc(heap_allocs, sizeof(void *) * max_alloc);
				if (!heap_allocs) {
					throw uabort("out of memory");
				}
			}
		
			pos = num_alloc;
		}
		
		heap_allocs[pos] = std::realloc(ptr, len);
		if (!heap_allocs[pos]) {
			throw uabort("out of memory");
		}
		
		if (!ptr) {
			num_alloc++;
		}
		
		return heap_allocs[pos];
	}
	
	
	void *malloc(size_t len) {
		return realloc(NULL, len);
	}
	
	
	void free(void *p) {
		throw uabort("not supported");
	}


	char *strdup(const char *text) {
		if (!text) {
			return nullptr;
		}
				
		char *ret = (char *) malloc(std::strlen(text) + 1);
		for (char *p = ret; /* noop */ ;p++, text++) {
			*p = *text;
			if (!*text) {
				break;
			}
		}
			
		return ret;
	}
};


const char *deployment::MAN_SEC_CORE   = "core:";
const char *deployment::MAN_SEC_KERNEL = "kernel:";
const char *deployment::MAN_SEC_BUILD  = "build:";
const char *deployment::MAN_SEC_INC    = "include:";


bool un::deploy( char *error, size_t len ) {
	return deployment().deploy(error, len);
}
