/*-------------------------------------------------------------------
| vi: set noet ts=4 sw=4 fenc=utf-8
| -------------------------------------------------------------------
| Copyright (c) 2025 Francis Henry Grolemund III and RealProven, LLC.
| SPDX-License-Identifier: LicenseRef-Unum-Commercial OR GPL-3.0-only
| -------------------------------------------------------------------*/

#ifndef UNUM_MANIFEST_H
#define UNUM_MANIFEST_H


// -- UNUM NAMESPACE
namespace un {


// TODO: the idea is that the manifest will be valid or invalid, storing its
// TODO: ...own error state that is self-contained for the context.


class manifest {
	// - read and return object, that may be an error-state
	// - requires that the manifest be formatted correctly and the files
	//   point to real things!
	// static manifest fread(const un::string root, const un::string file);
	
	// - return `true` if not in an error state
	// operator bool
	
	// - return error message if in an error state
	// const char const * error_msg();
	
	// - clear an active error state
	// void error_reset();
	
	// - return an array of source files in sorted order by priority
	// - a manifest::path is a string along with a time of last modification
	// array<const manifest::path> source_files()
	
	// - return an array of include directories in sorted order by priority
	// array<const manifest::path> include_dirs()
	
	// - add a file to the manifest
	// manifest &add(string file_path, string before_path, section_t section);
	// manifest &add(string file_path, path before, section_t section);
	
	// - remove a file from the manifest
	// manifest &remove(string item_path);
	// manifest &remove(path item);
	
	// - write manifest
	// manifest &fwrite();
	// manifest &fwrite(string path);

	
		// TODO: strings
		// TODO: file handling
		// TODO: testing


};



}

#endif /* UNUM_MANIFEST_H */
