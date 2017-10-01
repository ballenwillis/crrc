#pragma once
#include <Cutelyst/context.h>

namespace crrc
{
  namespace dao
  {
    struct AgreementDAO
    {
      QVariantList retrieveAll() const;
      QVariantList retrieveByInstitution( uint32_t id ) const;
      QVariant retrieve( const uint32_t id ) const;
      QByteArray contents( Cutelyst::Context* context, const uint32_t id ) const;
      uint32_t insert( Cutelyst::Context* context ) const;
      uint32_t update( Cutelyst::Context* context ) const;
      QVariantList search( Cutelyst::Context* context ) const;
      uint32_t remove( uint32_t id ) const;
      uint32_t saveProgram( Cutelyst::Context* context ) const;
    };
  }
}