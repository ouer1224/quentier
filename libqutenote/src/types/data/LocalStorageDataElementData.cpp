#include "LocalStorageDataElementData.h"

namespace qute_note {

LocalStorageDataElementData::LocalStorageDataElementData() :
    QSharedData(),
    m_localUid(QUuid::createUuid())
{}

LocalStorageDataElementData::~LocalStorageDataElementData()
{}

LocalStorageDataElementData::LocalStorageDataElementData(const LocalStorageDataElementData & other) :
    QSharedData(other),
    m_localUid(other.m_localUid)
{}

LocalStorageDataElementData::LocalStorageDataElementData(LocalStorageDataElementData && other) :
    QSharedData(std::move(other)),
    m_localUid(std::move(other.m_localUid))
{}

} // namespace qute_note
