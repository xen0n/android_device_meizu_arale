#ifndef ANDROID_MTK_RWTABLE_H
#define ANDROID_MTK_RWTABLE_H
#include <utils/Singleton.h>
#include <utils/KeyedVector.h>
#include <utils/RefBase.h>
#include <utils/RWLock.h>

namespace android {
template <typename KeyType, typename ValueType>
class RWTable {
public:
    RWTable() :
        mMap(NULL)
    {
    }

    sp<ValueType> operator[] (KeyType const key) {
        ssize_t idx = 0;
        sp<ValueType> value = NULL;
        {
            RWLock::AutoRLock l(mLock);
            idx = mMap.indexOfKey(key);
            if (idx >= 0) {
                value = mMap.editValueAt(idx);
            }
        }
        if (idx < 0) {
            value = createEntry();
            {
                RWLock::AutoWLock l(mLock);
                mMap.add(key, value);
            }
        }
        return value;
    }

    bool remove(KeyType const key) {
        RWLock::AutoWLock l(mLock);
        return mMap.removeItem(key) >= 0 ? true : false;
    }

    virtual ~RWTable() {
    }

protected:
    virtual sp<ValueType> createEntry() = 0;
    DefaultKeyedVector<KeyType, sp<ValueType> > mMap;
    RWLock mLock;
};

}; // namespace android
#endif // ANDROID_MTK_RWTABLE_H
