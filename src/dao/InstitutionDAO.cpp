#include "InstitutionDAO.h"
#include "ContactDAO.h"
#include "LogoDAO.h"
#include "InstitutionDesignationDAO.h"
#include "constants.h"
#include "model/Institution.h"

#include <mutex>
#include <unordered_map>

#include <QtCore/QLoggingCategory>
#include <QtCore/QStringBuilder>
#include <QtSql/QtSql>
#include <Cutelyst/Plugins/Utils/sql.h>

using crrc::model::Institution;
Q_LOGGING_CATEGORY( INSTITUTION_DAO, "crrc.dao.InstitutionDAO" )

namespace crrc
{
  namespace dao
  {
    static std::unordered_map<uint32_t, Institution::Ptr> institutions;
    static std::atomic_bool institutionsLoaded{ false };
    static std::mutex institutionMutex;

    QVariantList fromInstitutions()
    {
      QVariantList list;
      for ( const auto& iter : institutions ) list << asVariant( iter.second.get() );
      return list;
    }

    void loadInstitutions()
    {
      if ( institutionsLoaded.load() ) return;
      std::lock_guard<std::mutex> lock{ institutionMutex };
      if ( !institutions.empty() )
      {
        institutionsLoaded = true;
        return;
      }

      auto query = CPreparedSqlQueryThreadForDB(
        R"(
select institution_id, name, address, city, state, postal_code, country,
  website, logo_id, institution_type_id
from institutions
order by name
)", crrc::DATABASE_NAME );

      if ( query.exec() )
      {
        while ( query.next() )
        {
          auto institution = Institution::create( query );
          institutions[institution->getId()] = std::move( institution );
        }

        institutionsLoaded = true;
      }
      else
      {
        qWarning( INSTITUTION_DAO ) << 
          "Database error while loading institutions: " << 
          query.lastError().text();
      }
    }

    void bindInstitution( Cutelyst::Context* c, QSqlQuery& query )
    {
      query.bindValue( ":name", c->request()->param( "name" ) );
      query.bindValue( ":address", c->request()->param( "address" ) );
      query.bindValue( ":city", c->request()->param( "city" ) );
      query.bindValue( ":state", c->request()->param( "state" ) );
      query.bindValue( ":postalCode", c->request()->param( "postalCode" ) );
      query.bindValue( ":country", c->request()->param( "country", "USA" ) );
      query.bindValue( ":website", c->request()->param( "website" ) );
      query.bindValue( ":logo", c->request()->param( "logoId" ) );
      query.bindValue( ":instType", c->request()->param( "institutionTypeId" ) );
    }
  }
}

using crrc::dao::InstitutionDAO;

QVariantList InstitutionDAO::retrieveAll() const
{
  loadInstitutions();
  return fromInstitutions();
}

QVariantList InstitutionDAO::byDegree( const uint32_t degreeId ) const
{
  loadInstitutions();
  QSqlQuery query = CPreparedSqlQueryThreadForDB(
    R"(
select i.institution_id
from institutions i
inner join programs p on (p.degree_id = :degree and p.institution_id = i.institution_id)
order by i.name
)", crrc::DATABASE_NAME );
  query.bindValue( ":degree", degreeId );

  QVariantList list;

  if ( query.exec() )
  {
    while ( query.next() )
    {
      const auto institution = retrieve( query.value( 0 ).toUInt() );
      list << institution;
    }
  }
  else
  {
    qWarning( INSTITUTION_DAO ) << "Database error: " << query.lastError().text();
  }

  return list;
}

QVariant InstitutionDAO::retrieve( uint32_t id ) const
{
  loadInstitutions();
  const auto iter = institutions.find( id );

  return ( iter != institutions.end() ) ? asVariant( iter->second.get() ) : QVariant();
}

uint32_t InstitutionDAO::insert( Cutelyst::Context* context ) const
{
  loadInstitutions();
  QSqlQuery query = CPreparedSqlQueryThreadForDB(
R"(
insert into institutions
(name, address, city, state, postal_code, country, website, logo_id, institution_type_id)
values
(:name, :address, :city, :state, :postalCode, :country, :website, :logo, :instType)
)", crrc::DATABASE_NAME );
  bindInstitution( context, query );

  if ( !query.exec() )
  {
    qWarning( INSTITUTION_DAO ) << "Database error: " << query.lastError().text();
    context->stash()["error_msg"] = query.lastError().text();
    return 0;
  }

  const auto id = query.lastInsertId().toUInt();
  auto institution = Institution::create( context );
  institution->setId( id );
  std::lock_guard<std::mutex> lock{ institutionMutex };
  institutions[id] = std::move( institution );
  return id;
}

void InstitutionDAO::update( Cutelyst::Context* context ) const
{
  loadInstitutions();
  auto id = context->request()->param( "id" );
  auto query = CPreparedSqlQueryThreadForDB(
    R"(
update institutions set name=:name, address=:address, city=:city, state=:state,
  postal_code=:postalCode, country=:country, website=:website,
  logo_id=:logo, institution_type_id = :instType
where institution_id=:id
)", crrc::DATABASE_NAME );
  bindInstitution( context, query );
  query.bindValue( ":id", id.toInt() );

  if ( query.exec() )
  {
    const auto iid = id.toUInt();
    auto institution = Institution::create( context );
    std::lock_guard<std::mutex> lock{ institutionMutex };
    institutions[iid] = std::move( institution );
  }
  else
  {
    qWarning( INSTITUTION_DAO ) << "Database error: " << query.lastError().text();
    context->stash()["error_msg"] = query.lastError().text();
  }
}

QVariantList InstitutionDAO::search( Cutelyst::Context* context ) const
{
  loadInstitutions();
  const auto text = context->request()->param( "text", "" );
  const QString clause = "%" % text % "%";

  auto query = CPreparedSqlQueryThreadForDB(
   "select institution_id from institutions where name like :text order by name",
    DATABASE_NAME );

  query.bindValue( ":text", clause );
  QVariantList list;

  if ( query.exec() )
  {
    while ( query.next() )
    {
      auto institution = retrieve( query.value( 0 ).toUInt() );
      list.append( institution );
    }
  }
  else
  {
    qWarning( INSTITUTION_DAO ) << query.lastError().text();
    context->stash()["error_msg"] = query.lastError().text();
  }

  return list;
}

uint32_t InstitutionDAO::remove( uint32_t id ) const
{
  loadInstitutions();
  auto query = CPreparedSqlQueryThreadForDB(
    "delete from institutions where institution_id = :id", DATABASE_NAME );
  query.bindValue( ":id", id );

  if ( query.exec() )
  {
    const auto count = query.numRowsAffected();
    ContactDAO().removeInstitution( id );
    InstitutionDesignationDAO().remove( id );

    const auto iter = institutions.find( id );
    if ( iter != institutions.end() )
    {
      LogoDAO().remove( iter->second->getLogoId() );
    }

    std::lock_guard<std::mutex> lock{ institutionMutex };
    institutions.erase( id );
    return count;
  }

  qWarning( INSTITUTION_DAO ) << query.lastError().text();
  return 0;
}

bool InstitutionDAO::isUnique( const QString& name, const QString& city ) const
{
  loadInstitutions();
  for ( const auto& iter : institutions )
  {
    if ( iter.second->getName() == name &&
      iter.second->getCity() == city ) return false;
  }

  return true;
}
