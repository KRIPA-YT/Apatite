#pragma once

template <typename T> class ManagedSingleton {
public:
	static void createInstance() {
		if (managedSingleton != nullptr) {
			//ToDo: Throw an assert error here
			return;
		}
		managedSingleton = new T();
	}

	static void destroyInstance() {
		if (managedSingleton == nullptr) {
			return;
		}
		delete managedSingleton;
	}

	static const char* getInstanceName() {
		return "UNNAMED_SINGLETON_INSTACE";
	}

	static T* instance() {
		return managedSingleton ? managedSingleton : nullptr;
	}

private:
	static T* managedSingleton;
};

template <typename T> T* ManagedSingleton<T>::managedSingleton = nullptr;
