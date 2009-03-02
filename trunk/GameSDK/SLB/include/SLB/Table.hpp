/*
    SLB - Simple Lua Binder
    Copyright (C) 2007 Jose L. Hidalgo Valiño (PpluX)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

	Jose L. Hidalgo (www.pplux.com)
	pplux@pplux.com
*/

#ifndef __SLB_TABLE__
#define __SLB_TABLE__

#include "Export.hpp"
#include "Object.hpp"
#include "ref_ptr.hpp"
#include <typeinfo>
#include <map>
#include <string>
#include <list>
#include "lua.hpp"

namespace SLB {

	class SLB_EXPORT Table : public virtual Object {
	public:
		typedef std::map< std::string, ref_ptr<Object> > Elements;
		Table(const std::string &separator = "", bool cacheable = false);

		void erase(const std::string &);
		Object* get(const std::string &);
		void getElements( std::list< const char* > &l )const;
		void set(const std::string &, Object*);

		bool isCacheable() { return _cacheable; }

		// [ -2, 0, - ] pops two elements (key, value) from the top and pushes it into
		// the cache. If _cacheable == false this won't make much sense, but you can use
		// it anyway (you can recover the values with getCache).
		void setCache(lua_State *L);

		// [ -1, +1, - ] will pop a key, and push the value or nil if not found.
		void getCache(lua_State *L);

	protected:
		virtual ~Table();

		Object* rawGet(const std::string &);
		void rawSet(const std::string &, Object*);
		
		void pushImplementation(lua_State *);

		/** will try to find the object, if not present will return -1. If this
		 * function is not overriden not finding an object will raise an error
		 * It is highly recommended to call this method in subclasses of Table
		 * first.*/
		virtual int __index(lua_State *L); 
		virtual int __newindex(lua_State *L);

		virtual int __call(lua_State *L);
		virtual int __garbageCollector(lua_State *L);
		virtual int __tostring(lua_State *L);

		Elements _elements;
		/** this function returns the index where to find the cache table that
		 * __index method uses if _cacheable is true. This method must NOT be called
		 * outside metamethod's implementation. */
		int cacheTableIndex() { return lua_upvalueindex(1); }

	private:
		typedef std::pair<Table*,const std::string> TableFind;
		typedef int (Table::*TableMember)(lua_State*);

		int __indexProxy(lua_State *L);
		static int __meta(lua_State*);
		void pushMeta(lua_State *L, TableMember) const;

		TableFind getTable(const std::string &key, bool create);

		bool _cacheable;
		std::string _separator;

		Table(const Table&);
		Table& operator=(const Table&);
	};

	//--------------------------------------------------------------------
	// Inline implementations:
	//--------------------------------------------------------------------
		
}


#endif
