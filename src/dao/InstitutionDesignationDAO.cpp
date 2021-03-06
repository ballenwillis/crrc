﻿#include "InstitutionDesignationDAO.h"
#include "constants.h"
#include "model/InstitutionDesignation.h"

#include <QtCore/QLoggingCategory>
#include <QtSql/QtSql>
#include <Cutelyst/Plugins/Utils/sql.h>

using crrc::model::InstitutionDesignation;
Q_LOGGING_CATEGORY( INSTITUTION_DESIGNATION_DAO, "crrc.dao.InstitutionDesignationDAO" )

namespace crrc
{
  namespace dao
  {
    void bindInstitutionDesignation( Cutelyst::Context* c, QSqlQuery& query )
    {
      query.bindValue( ":iid", c->request()->param( "institutionId" ).toUInt() );
      query.bindValue( ":did", c->request()->param( "designationId" ).toUInt() );
      query.bindValue( ":expiration", c->request()->param( "expiration" ) );
    }
  }
}

using crrc::dao::InstitutionDesignationDAO;

QVariantList InstitutionDesignationDAO::retrieveAll( Cutelyst::Context* context ) const
{
  QVariantList list;

  auto query = CPreparedSqlQueryThreadForDB( 
    R"(
select institution_id, designation_id, expiration
from institution_designations
order by designation_id
)",
    crrc::DATABASE_NAME );

  if ( query.exec() )
  {
    while ( query.next() )
    {
      list << asVariant( InstitutionDesignation::create( context, query ) );
    }
  }
  else qWarning( INSTITUTION_DESIGNATION_DAO ) << query.lastError().text();

  return list;
}

QVariantList InstitutionDesignationDAO::retrieve(
  Cutelyst::Context* context, uint32_t institutionId ) const
{
  QVariantList list;

  auto query = CPreparedSqlQueryThreadForDB( 
    R"(
select institution_id, designation_id, expiration
from institution_designations
where institution_id = :iid
)",
    crrc::DATABASE_NAME );
  query.bindValue( ":iid", institutionId );
  if ( query.exec() )
  {
    while ( query.next() )
    {
      list << asVariant( InstitutionDesignation::create( context, query ) );
    }
  }
  else qWarning( INSTITUTION_DESIGNATION_DAO ) << query.lastError().text();

  return list;
}

uint32_t InstitutionDesignationDAO::save( Cutelyst::Context* context ) const
{
  const auto result = update( context );
  return result ? result : insert( context );
}

uint32_t InstitutionDesignationDAO::remove( Cutelyst::Context* context ) const
{
  auto query = CPreparedSqlQueryThreadForDB( 
    "delete from institution_designations where institution_id = :iid and designation_id = :did",
    crrc::DATABASE_NAME );
  query.bindValue( ":iid", context->request()->param( "institutionId" ).toUInt() );
  query.bindValue( ":did", context->request()->param( "designationId" ).toUInt() );

  if ( query.exec() ) return query.numRowsAffected();

  qWarning( INSTITUTION_DESIGNATION_DAO ) << query.lastError().text();
  context->setStash( "error_msg", query.lastError().text() );
  return 0;
}

uint32_t InstitutionDesignationDAO::remove( uint32_t institutionId ) const
{
  auto query = CPreparedSqlQueryThreadForDB( 
    "delete from institution_designations where institution_id = :iid",
    crrc::DATABASE_NAME );
  query.bindValue( ":iid", institutionId );
  if ( query.exec() ) return query.numRowsAffected();

  qWarning( INSTITUTION_DESIGNATION_DAO ) << query.lastError().text();
  return 0;
}

uint32_t InstitutionDesignationDAO::update( Cutelyst::Context* context ) const
{
  auto query = CPreparedSqlQueryThreadForDB( 
    R"(
update institution_designations set expiration = :expiration
where institution_id = :iid
and designation_id = :did
)",
    crrc::DATABASE_NAME );
  bindInstitutionDesignation( context, query );
  if ( query.exec() ) return query.numRowsAffected();

  context->setStash( "error_msg", query.lastError().text() );
  qWarning( INSTITUTION_DESIGNATION_DAO ) << query.lastError().text();
  return 0;
}

uint32_t InstitutionDesignationDAO::insert( Cutelyst::Context* context ) const
{
  auto query = CPreparedSqlQueryThreadForDB( 
    R"(
insert into institution_designations
(institution_id, designation_id, expiration)
values
(:iid, :did, :expiration)
)",
    crrc::DATABASE_NAME );
  bindInstitutionDesignation( context, query );
  if ( query.exec() ) return query.numRowsAffected();

  context->setStash( "error_msg", query.lastError().text() );
  qWarning( INSTITUTION_DESIGNATION_DAO ) << query.lastError().text();
  return 0;
}
