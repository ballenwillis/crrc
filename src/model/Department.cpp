﻿#include "Department.h"
#include "Institution.h"
#include "dao/InstitutionDAO.h"

using crrc::model::Department;

Department::Ptr Department::create( QSqlQuery& query )
{
  auto ptr = std::make_unique<Department>();
  ptr->id = query.value( 0 ).toUInt();
  ptr->name = query.value( 1 ).toString();
  ptr->prefix = query.value( 2 ).toString();
  ptr->institutionId = query.value( 3 ).toUInt();
  return ptr;
}

Department::Ptr Department::create( Cutelyst::Context* context )
{
  auto ptr = std::make_unique<Department>();

  const auto& id = context->request()->param( "id" );
  if ( !id.isEmpty() ) ptr->id = id.toUInt();
  
  ptr->name = context->request()->param( "name" );
  ptr->prefix = context->request()->param( "prefix" );
  ptr->institutionId = context->request()->param( "institution" ).toUInt();
  return ptr;
}

QVariant Department::getInstitution() const
{
  return ( institutionId > 0 ) ? dao::InstitutionDAO().retrieve( institutionId ) : QVariant();
}

QJsonObject crrc::model::toJson( const Department& department )
{
  QJsonObject obj;
  obj.insert( "id", static_cast<int>( department.getId() ) );
  obj.insert( "name", department.getName() );
  obj.insert( "prefix", department.getPrefix() );

  if ( department.getInstitutionId() )
  {
    const auto ptr = Institution::from( department.getInstitution() );
    obj.insert( "institution", ptr ? toJson( *ptr, true ) : QJsonObject{} );
  }

  return obj;
}
