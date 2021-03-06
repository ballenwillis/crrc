#include "ContactTest.h"

#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

namespace crrc
{
  namespace contacttest
  {
    static uint32_t id = 0;
    static int32_t institutionId = 0;
    static const QString institutionName{ "Unit Test Institution" };
    static const QString institutionCity{ "Unit Test City" };
    static const QString modified = "ddd";
  }
}

using crrc::ContactTest;

void ContactTest::initTestCase()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString{ "http://localhost:3000/institutions/edit" };
  const auto data = QString( "name=%1&city=%2" ).
      arg( contacttest::institutionName ).arg( contacttest::institutionCity );
  auto reply = put( url, data, &mgr, &eventLoop, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error creating new institution" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty JSON response for create institution" );
  QVERIFY2( obj["id"].toInt(), "Create institution returned invalid id" );
  contacttest::institutionId = obj["id"].toInt();

  logout( &mgr, &eventLoop, &req );
}

void ContactTest::create()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString{ "http://localhost:3000/contacts/edit" };
  const auto data = QString( "name=%1&workEmail=%2&username=%3&password=%4&role=%5&title=%6&institution=%7" )
      .arg( name ).arg( workEmail ).arg( username ).arg( password )
      .arg( roleId ).arg( title ).arg( contacttest::institutionId );
  auto reply = put( url, data, &mgr, &eventLoop, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error creating new contact" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty JSON response for create contact" );
  QVERIFY2( obj["id"].toInt(), "Create contact returned invalid id" );
  contacttest::id = obj["id"].toInt();

  logout( &mgr, &eventLoop, &req );
}

void ContactTest::retrieveAll()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString{ "http://localhost:3000/contacts" };
  const auto reply = post( url, "", &mgr, &eventLoop, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error retrieving contact list" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto array = doc.array();
  QVERIFY2( !array.isEmpty(), "Empty json response for contact list" );

  logout( &mgr, &eventLoop, &req );
}

void ContactTest::retrieve()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString( "http://localhost:3000/contacts/id/%1/data" ).arg( contacttest::id );
  const auto reply = get( url, &mgr, &eventLoop, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error retrieving contact" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty JSON response for contact id" );
  QVERIFY2( contacttest::id == obj["id"].toInt(), "Json response returned invalid id" );
  QVERIFY2( name == obj["name"].toString(), "Json response returned invalid name" );
  QVERIFY2( workEmail == obj["workEmail"].toString(), "Json response returned invalid workEmail" );
  QVERIFY2( title == obj["title"].toString(), "Json response returned invalid workEmail" );
  QVERIFY2( contacttest::institutionId == obj["institution"].toObject()["id"].toInt(), "Json response returned invalid institution" );
  qDebug() << obj["institution"];

  logout( &mgr, &eventLoop, &req );
}

void ContactTest::invalid()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString{ "http://localhost:3000/contacts/id/0/data" };
  const auto reply = get( url, &mgr, &eventLoop, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error retrieving invalid contact" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( obj.isEmpty(), "Non-Empty JSON response for invalid contact id" );

  logout( &mgr, &eventLoop, &req );
}

void ContactTest::checkUsername()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString{ "http://localhost:3000/contacts/isUsernameAvailable/?username=%1" }.arg( username );
  const auto reply = get( url, &mgr, &eventLoop, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error retrieving contact" );
  const QString text = reply->readAll();
  QVERIFY2( !text.isEmpty(), "Empty response for username" );
  QVERIFY2( "false" == text, "Json response returned invalid name" );

  logout( &mgr, &eventLoop, &req );
}

void ContactTest::checkInvalidUsername()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString( "http://localhost:3000/contacts/isUsernameAvailable/?username=blahblah1" );
  const auto reply = get( url, &mgr, &eventLoop, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error retrieving contact" );
  const QString text = reply->readAll();
  QVERIFY2( !text.isEmpty(), "Empty response for username" );
  QVERIFY2( "true" == text, "Json response returned invalid name" );

  logout( &mgr, &eventLoop, &req );
}

void ContactTest::index()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString{ "http://localhost:3000/contacts" };
  const auto data = QString{};
  const auto reply = post( url, data, &mgr, &eventLoop, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error retrieving contact list" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto arr = doc.array();
  QVERIFY2( !arr.isEmpty(), "Empty JSON response for contact list" );

  bool found = false;
  for ( const auto& obj : arr )
  {
    if ( contacttest::id == obj.toObject()["id"].toInt() )
    {
      found = true;
      break;
    }
  }

  QVERIFY2( found, "Newly created contact not returned in list" );

  logout( &mgr, &eventLoop, &req );
}

void ContactTest::loginBeforeEdit()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( username, password, &mgr, &eventLoop, &req );

  auto reply = get( "http://localhost:3000/", &mgr, &eventLoop, &req );
  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error logging into server" );
  const auto code = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
  QVERIFY2( 200 == code, "Root page not served" );

  logout( &mgr, &eventLoop, &req );
}

void ContactTest::update()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString{ "http://localhost:3000/contacts/edit" };
  const auto phone = "1 111-222-3333";
  const auto data = QString( "id=%1&name=%2&workEmail=%3&homePhone=%4&username=%5&institution=%6" )
      .arg( contacttest::id ).arg( name ).arg( workEmail )
      .arg( phone ).arg( username ).arg( contacttest::institutionId );
  const auto reply = put( url, data, &mgr, &eventLoop, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error updating contact" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty JSON response for create contact" );
  QVERIFY2( contacttest::id == obj["id"].toInt(), "Json response returned invalid id" );
  QVERIFY2( phone == obj["homePhone"].toString(), "Json response returned invalid phone" );

  logout( &mgr, &eventLoop, &req );
}

void ContactTest::loginAfterEdit()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( username, password, &mgr, &eventLoop, &req );

  auto reply = get( "http://localhost:3000/", &mgr, &eventLoop, &req );
  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error logging into server" );
  const auto code = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
  QVERIFY2( 200 == code, "Root page not served" );

  logout( &mgr, &eventLoop, &req );
}

void ContactTest::byInstitution()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( username, password, &mgr, &eventLoop, &req );

  const auto url = QString{ "http://localhost:3000/contacts/search" };
  const auto data = QString{ "institution=%1" }.arg( contacttest::institutionId );
  const auto reply = put( url, data, &mgr, &eventLoop, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error retrieving contact list by institution" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto array = doc.array();
  QVERIFY2( !array.isEmpty(), "Empty json response for contact list by institution" );

  logout( &mgr, &eventLoop, &req );
}

void ContactTest::search()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( username, password, &mgr, &eventLoop, &req );

  const auto url = QString{ "http://localhost:3000/contacts/search" };
  const auto data = QString{ "text=%1" }.arg( name );
  const auto reply = put( url, data, &mgr, &eventLoop, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error searching contacts" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto array = doc.array();
  QVERIFY2( !array.isEmpty(), "Empty json response for search" );

  logout( &mgr, &eventLoop, &req );
}

void ContactTest::remove()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString{ "http://localhost:3000/contacts/remove" };
  const auto data = QString( "id=%1" ).arg( contacttest::id );
  const auto reply = put( url, data, &mgr, &eventLoop, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error deleting contact" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty JSON response for delete contact" );
  QVERIFY2( obj["status"].toBool(), "Remove contact returned false status" );
  QVERIFY2( obj["count"].toInt(), "Remove contact returned invalid count" );

  logout( &mgr, &eventLoop, &req );
}

void ContactTest::readDeleted()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString{ "http://localhost:3000/contacts/id/%1/data" }.arg( contacttest::id );
  const auto reply = get( url, &mgr, &eventLoop, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error retrieving deleted contact" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( obj.isEmpty(), "Non-Empty JSON response for deleted contact id" );

  logout( &mgr, &eventLoop, &req );
}

void ContactTest::cleanupTestCase()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString{ "http://localhost:3000/institutions/remove" };
  const auto data = QString( "id=%1" ).arg( contacttest::institutionId );
  const auto reply = put( url, data, &mgr, &eventLoop, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error deleting institution" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty JSON response for delete institution" );
  QVERIFY2( obj["status"].toBool(), "Remove institution returned false status" );
  QVERIFY2( obj["count"].toInt(), "Remove institution returned invalid count" );

  logout( &mgr, &eventLoop, &req );
}

