#pragma once

#include <AK/AKString.h>
#include <AK/Badge.h>
#include <AK/InlineLinkedList.h>
#include <AK/RefCounted.h>
#include <AK/RefPtr.h>

class Inode;
class VFS;

// FIXME: Custody needs some locking.

class Custody : public RefCounted<Custody>
    , public InlineLinkedListNode<Custody> {
public:
    static Custody* get_if_cached(Custody* parent, const StringView& name);
    static NonnullRefPtr<Custody> get_or_create(Custody* parent, const StringView& name, Inode&);
    static NonnullRefPtr<Custody> create(Custody* parent, const StringView& name, Inode& inode)
    {
        return adopt(*new Custody(parent, name, inode));
    }

    ~Custody();

    Custody* parent() { return m_parent.ptr(); }
    const Custody* parent() const { return m_parent.ptr(); }
    Inode& inode() { return *m_inode; }
    const Inode& inode() const { return *m_inode; }
    const String& name() const { return m_name; }
    String absolute_path() const;

    bool is_deleted() const { return m_deleted; }
    bool is_mounted_on() const { return m_mounted_on; }

    void did_delete(Badge<VFS>);
    void did_mount_on(Badge<VFS>);
    void did_rename(Badge<VFS>, const String& name);

    // For InlineLinkedListNode.
    Custody* m_next { nullptr };
    Custody* m_prev { nullptr };

private:
    Custody(Custody* parent, const StringView& name, Inode&);

    RefPtr<Custody> m_parent;
    String m_name;
    NonnullRefPtr<Inode> m_inode;
    bool m_deleted { false };
    bool m_mounted_on { false };
};
