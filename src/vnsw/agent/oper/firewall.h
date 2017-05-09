/*
 * Copyright (c) 2016 Juniper Networks, Inc. All rights reserved.
 */

#ifndef vnsw_agent_firewall_hpp
#define vnsw_agent_firewall_hpp

#include <cmn/agent_cmn.h>
#include <cmn/agent.h>
#include <agent_types.h>
#include <oper/oper_db.h>

using namespace boost::uuids;
using namespace std;

struct TagKey : public AgentOperDBKey {
    TagKey(uuid tag_uuid) : AgentOperDBKey(), tag_uuid_(tag_uuid) {} ;
    virtual ~TagKey() { };

    uuid tag_uuid_;
};

struct TagData : public AgentOperDBData {
    TagData(Agent *agent, IFMapNode *node, const uint32_t &tag_id):
                   AgentOperDBData(agent, node), tag_id_(tag_id){
    }
    virtual ~TagData() { }

    uint32_t tag_id_;
};

class TagEntry : AgentRefCount<TagEntry>, public AgentOperDBEntry {
public:
    TagEntry(uuid tag_uuid, uint32_t tag_id) : tag_uuid_(tag_uuid), tag_id_(tag_id) {};
    TagEntry(uuid tag_uuid) : tag_uuid_(tag_uuid) { };
    virtual ~TagEntry() { };

    virtual bool IsLess(const DBEntry &rhs) const;
    virtual KeyPtr GetDBRequestKey() const;
    virtual void SetKey(const DBRequestKey *key);
    virtual string ToString() const;

    const uuid &GetTagUuid() const {return tag_uuid_;};
    const uint32_t &GetTagId() const {return tag_id_;};

    uint32_t GetRefCount() const {
        return AgentRefCount<TagEntry>::GetRefCount();
    }
    bool DBEntrySandesh(Sandesh *sresp, std::string &name) const;

private:
    friend class TagTable;
    uuid tag_uuid_;
    uint32_t tag_id_;
    DISALLOW_COPY_AND_ASSIGN(TagEntry);
};

class TagTable : public AgentOperDBTable {
public:
    static const uint32_t kInvalidTagId = 0;
    TagTable(DB *db, const std::string &name) : AgentOperDBTable(db, name) { }
    virtual ~TagTable() { }

    virtual std::auto_ptr<DBEntry> AllocEntry(const DBRequestKey *k) const;
    virtual size_t Hash(const DBEntry *entry) const {return 0;};
    virtual size_t  Hash(const DBRequestKey *key) const {return 0;};

    virtual DBEntry *OperDBAdd(const DBRequest *req);
    virtual bool OperDBOnChange(DBEntry *entry, const DBRequest *req);
    virtual bool OperDBDelete(DBEntry *entry, const DBRequest *req);

    virtual bool IFNodeToReq(IFMapNode *node, DBRequest &req,
            const boost::uuids::uuid &u);
    virtual bool IFNodeToUuid(IFMapNode *node, boost::uuids::uuid &u);
    bool ProcessConfig(IFMapNode *node, DBRequest &req,
            const boost::uuids::uuid &u);

    static DBTableBase *CreateTable(DB *db, const std::string &name);
    static TagTable *GetInstance() {return tag_table_;};

private:
    static TagTable* tag_table_;
    bool ChangeHandler(DBEntry *entry, const DBRequest *req);
    DISALLOW_COPY_AND_ASSIGN(TagTable);
};

#endif // vnsw_agent_firewall_hpp
