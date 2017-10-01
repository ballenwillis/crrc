﻿#pragma once
#include "BlobItem.h"

#include <memory>

namespace crrc
{
  namespace model
  {
    class Logo : public BlobItem
    {
      Q_OBJECT

    public:
      using Ptr = std::unique_ptr<Logo>;

      explicit Logo( QObject* parent = nullptr ) : BlobItem( parent ) {}
      ~Logo() = default;
    };

    inline QVariant asVariant( const Logo* logo )
    {
      return QVariant::fromValue<QObject*>( const_cast<Logo*>( logo ) );
    }
  }
}
