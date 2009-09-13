class Extensible;
class Module;

/** Class represnting an extension of some object
 */
class CoreExport ExtensionItem
{
 public:
	const std::string key;
	Module* const owner;
	ExtensionItem(const std::string& key, Module* owner);
	/** Serialize this item into a string */
	virtual std::string serialize(Module* requestor, const Extensible* container, void* item) = 0;
	/** Convert the string form back into an item */
	virtual void unserialize(Module* requestor, Extensible* container, const std::string& value) = 0;
	/** Free the item */
	virtual void free(void* item) = 0;

 protected:
	/** Get the item from the internal map */
	void* get_raw(const Extensible* container);
	/** Set the item in the internal map; returns old value */
	void* set_raw(Extensible* container, void* value);
	/** Remove the item from the internal map; returns old value */
	void* unset_raw(Extensible* container);
};

/** A private data store for an Extensible class */
typedef std::map<std::string,void*> ExtensibleStore;

/** class Extensible is the parent class of many classes such as User and Channel.
 * class Extensible implements a system which allows modules to 'extend' the class by attaching data within
 * a map associated with the object. In this way modules can store their own custom information within user
 * objects, channel objects and server objects, without breaking other modules (this is more sensible than using
 * a flags variable, and each module defining bits within the flag as 'theirs' as it is less prone to conflict and
 * supports arbitary data storage).
 */
class CoreExport Extensible : public classbase
{
	/** Private data store.
	 * Holds all extensible metadata for the class.
	 */
	ExtensibleStore extensions;
	typedef std::map<std::string, ExtensionItem*> ExtensibleTypes;
	static ExtensibleTypes extension_types;
 public:
	/**
	 * Get the extension items for iteraton (i.e. for metadata sync during netburst)
	 */
	inline const ExtensibleStore& GetExtList() const { return extensions; }
	static inline const ExtensibleTypes& GetTypeList() { return extension_types; }
	static inline ExtensionItem* GetItem(const std::string& name)
	{
		ExtensibleTypes::iterator i = extension_types.find(name);
		if (i == extension_types.end())
			return NULL;
		return i->second;
	}

	virtual ~Extensible();

	static bool Register(ExtensionItem* item);
	static void UnRegister(Module* module);
	
	// Friend access for the protected getter/setter
	friend class ExtensionItem;
};

/** Base class for items that are NOT synchronized between servers */
class CoreExport LocalExtItem : public ExtensionItem
{
 public:
	LocalExtItem(const std::string& key, Module* owner);
	// this is deliberately NOT virtual; don't subclass LocalExtItem if you want to sync data!
	std::string serialize(Module* requestor, const Extensible* container, void* item);
	void unserialize(Module* requestor, Extensible* container, const std::string& value);
	virtual void free(void* item) = 0;
};

template<typename T>
class CoreExport SimpleExtItem : public LocalExtItem
{
 public:
	SimpleExtItem(const std::string& Key, Module* parent) : LocalExtItem(Key, parent)
	{
	}

	inline T* get(const Extensible* container)
	{
		return static_cast<T*>(get_raw(container));
	}

	inline T* getNew(Extensible* container)
	{
		T* ptr = get(container);
		if (!ptr)
		{
			ptr = new T;
			set_raw(container, ptr);
		}
		return ptr;
	}

	inline void set(Extensible* container, const T& value)
	{
		T* ptr = new T(value);
		T* old = static_cast<T*>(set_raw(container, ptr));
		delete old;
	}

	inline void set(Extensible* container, T* value)
	{
		T* old = static_cast<T*>(set_raw(container, value));
		delete old;
	}

	inline void unset(Extensible* container)
	{
		T* old = static_cast<T*>(unset_raw(container));
		delete old;
	}

	virtual void free(void* item)
	{
		delete static_cast<T*>(item);
	}
};

typedef SimpleExtItem<std::string> LocalStringExt;

class CoreExport LocalIntExt : public LocalExtItem
{
 public:
	LocalIntExt(const std::string& key, Module* owner);
	intptr_t get(const Extensible* container);
	intptr_t set(Extensible* container, intptr_t value);
	void free(void* item);
};

class CoreExport StringExtItem : public ExtensionItem
{
 public:
	StringExtItem(const std::string& key, Module* owner);
	std::string* get(const Extensible* container);
	std::string serialize(Module* requestor, const Extensible* container, void* item);
	void unserialize(Module* requestor, Extensible* container, const std::string& value);
	void set(Extensible* container, const std::string& value);
	void unset(Extensible* container);
	void free(void* item);
};