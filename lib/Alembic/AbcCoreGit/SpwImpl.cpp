//-*****************************************************************************
//
// Copyright (c) 2014,
//
// All rights reserved.
//
//-*****************************************************************************

#include <Alembic/AbcCoreGit/SpwImpl.h>
#include <Alembic/AbcCoreGit/CpwImpl.h>
#include <Alembic/AbcCoreGit/WriteUtil.h>
#include <Alembic/AbcCoreGit/Utils.h>

namespace Alembic {
namespace AbcCoreGit {
namespace ALEMBIC_VERSION_NS {

//-*****************************************************************************
SpwImpl::SpwImpl( AbcA::CompoundPropertyWriterPtr iParent,
                  GitGroupPtr iGroup,
                  PropertyHeaderPtr iHeader,
                  size_t iIndex ) :
    m_parent( iParent ), m_header( iHeader ), m_group( iGroup ),
    m_index( iIndex )
{
    if (iGroup)
        TRACE("SpwImpl::SpwImpl(parent, group:" << iGroup->repr() << ", header, index:" << iIndex << ")");
    else
        TRACE("SpwImpl::SpwImpl(parent, group:NULL, header, index:" << iIndex << ")");

    ABCA_ASSERT( m_parent, "Invalid parent" );
    ABCA_ASSERT( m_header, "Invalid property header" );
    ABCA_ASSERT( m_group, "Invalid group" );

    if ( m_header->header.getPropertyType() != AbcA::kScalarProperty )
    {
        ABCA_THROW( "Attempted to create a ScalarPropertyWriter from a "
                    "non-scalar property type" );
    }
}


//-*****************************************************************************
SpwImpl::~SpwImpl()
{
    AbcA::ArchiveWriterPtr archive = m_parent->getObject()->getArchive();

    index_t maxSamples = archive->getMaxNumSamplesForTimeSamplingIndex(
            m_header->timeSamplingIndex );

    Util::uint32_t numSamples = m_header->nextSampleIndex;

    // a constant property, we wrote the same sample over and over
    if ( m_header->lastChangedIndex == 0 && numSamples > 0 )
    {
        numSamples = 1;
    }

    if ( maxSamples < numSamples )
    {
        archive->setMaxNumSamplesForTimeSamplingIndex(
            m_header->timeSamplingIndex, numSamples );
    }

    Util::SpookyHash hash;
    hash.Init(0, 0);
    HashPropertyHeader( m_header->header, hash );

    // mix in the accumulated hash if we have samples
    if ( numSamples != 0 )
    {
        hash.Update( m_hash.d, 16 );
    }

    Util::uint64_t hash0, hash1;
    hash.Final( &hash0, &hash1 );
    Util::shared_ptr< CpwImpl > parent =
        Alembic::Util::dynamic_pointer_cast< CpwImpl,
            AbcA::CompoundPropertyWriter > ( m_parent );
    UNIMPLEMENTED("CpwImpl::fillHash()");
#if 0
    parent->fillHash( m_index, hash0, hash1 );
#endif
}

//-*****************************************************************************
void SpwImpl::setFromPreviousSample()
{

    // Make sure we aren't writing more samples than we have times for
    // This applies to acyclic sampling only
    ABCA_ASSERT(
        !m_header->header.getTimeSampling()->getTimeSamplingType().isAcyclic()
        || m_header->header.getTimeSampling()->getNumStoredTimes() >
        m_header->nextSampleIndex,
        "Can not set more samples than we have times for when using "
        "Acyclic sampling." );

    ABCA_ASSERT( m_header->nextSampleIndex > 0,
        "Can't set from previous sample before any samples have been written" );

    UNIMPLEMENTED("WrittenSampleIDPtr m_previousWrittenSampleID");
#if 0
    Util::Digest digest = m_previousWrittenSampleID->getKey().digest;
    Util::SpookyHash::ShortEnd(m_hash.words[0], m_hash.words[1],
                               digest.words[0], digest.words[1]);
#endif /* 0 */
    m_header->nextSampleIndex ++;
}

//-*****************************************************************************
void SpwImpl::setSample( const void *iSamp )
{
    // Make sure we aren't writing more samples than we have times for
    // This applies to acyclic sampling only
    ABCA_ASSERT(
        !m_header->header.getTimeSampling()->getTimeSamplingType().isAcyclic()
        || m_header->header.getTimeSampling()->getNumStoredTimes() >
        m_header->nextSampleIndex,
        "Can not write more samples than we have times for when using "
        "Acyclic sampling." );

    AbcA::ArraySample samp( iSamp, m_header->header.getDataType(),
                            AbcA::Dimensions(1) );

     // The Key helps us analyze the sample.
     AbcA::ArraySample::Key key = samp.getKey();

     // mask out the non-string POD since Git can safely share the same data
     // even if it originated from a different POD
     // the non-fixed sizes of our strings (plus added null characters) makes
     // determing the sie harder so strings are handled seperately
    if ( key.origPOD != Alembic::Util::kStringPOD &&
         key.origPOD != Alembic::Util::kWstringPOD )
    {
        key.origPOD = Alembic::Util::kInt8POD;
        key.readPOD = Alembic::Util::kInt8POD;
    }

    // We need to write the sample
    UNIMPLEMENTED("WrittenSampleIDPtr m_previousWrittenSampleID");
#if 0
    if ( m_header->nextSampleIndex == 0  ||
        !( m_previousWrittenSampleID &&
            key == m_previousWrittenSampleID->getKey() ) )
    {

        // we only need to repeat samples if this is not the first change
        if (m_header->firstChangedIndex != 0)
        {
            // copy the samples from after the last change to the latest index
            for ( index_t smpI = m_header->lastChangedIndex + 1;
                smpI < m_header->nextSampleIndex; ++smpI )
            {
                assert( smpI > 0 );
                CopyWrittenData( m_group, m_previousWrittenSampleID );
            }
        }

        // Write this sample, which will update its internal
        // cache of what the previously written sample was.
        AbcA::ArchiveWriterPtr awp = this->getObject()->getArchive();

        // Write the sample.
        // This distinguishes between string, wstring, and regular arrays.
        m_previousWrittenSampleID =
            WriteData( GetWrittenSampleMap( awp ), m_group, samp, key );

        if (m_header->firstChangedIndex == 0)
        {
            m_header->firstChangedIndex = m_header->nextSampleIndex;
        }
        // this index is now the last change
        m_header->lastChangedIndex = m_header->nextSampleIndex;
    }

    if ( m_header->nextSampleIndex == 0 )
    {
        m_hash = m_previousWrittenSampleID->getKey().digest;
    }
    else
    {
        Util::Digest digest = m_previousWrittenSampleID->getKey().digest;
        Util::SpookyHash::ShortEnd( m_hash.words[0], m_hash.words[1],
                                    digest.words[0], digest.words[1] );
    }
#endif /* 0 */

    m_header->nextSampleIndex ++;
}

//-*****************************************************************************
AbcA::ScalarPropertyWriterPtr SpwImpl::asScalarPtr()
{
    return shared_from_this();
}

//-*****************************************************************************
size_t SpwImpl::getNumSamples()
{
    return ( size_t )m_header->nextSampleIndex;
}

//-*****************************************************************************
void SpwImpl::setTimeSamplingIndex( Util::uint32_t iIndex )
{
    // will assert if TimeSamplingPtr not found
    AbcA::TimeSamplingPtr ts =
        m_parent->getObject()->getArchive()->getTimeSampling( iIndex );

    ABCA_ASSERT( !ts->getTimeSamplingType().isAcyclic() ||
        ts->getNumStoredTimes() >= m_header->nextSampleIndex,
        "Already have written more samples than we have times for when using "
        "Acyclic sampling." );

    m_header->header.setTimeSampling(ts);
    m_header->timeSamplingIndex = iIndex;
}

//-*****************************************************************************
const AbcA::PropertyHeader & SpwImpl::getHeader() const
{
    ABCA_ASSERT( m_header, "Invalid header" );
    return m_header->header;
}

//-*****************************************************************************
AbcA::ObjectWriterPtr SpwImpl::getObject()
{
    ABCA_ASSERT( m_parent, "Invalid parent" );
    return m_parent->getObject();
}

//-*****************************************************************************
AbcA::CompoundPropertyWriterPtr SpwImpl::getParent()
{
    ABCA_ASSERT( m_parent, "Invalid parent" );
    return m_parent;
}

} // End namespace ALEMBIC_VERSION_NS
} // End namespace AbcCoreGit
} // End namespace Alembic
