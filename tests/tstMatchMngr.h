/* 
 * File:   tstTeamMngr.h
 * Author: volker
 *
 * Created on March 2, 2014, 3:46 PM
 */

#ifndef TSTMATCHMNGR_H
#define	TSTMATCHMNGR_H

#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "DatabaseTestScenario.h"

class tstMatchMngr : public DatabaseTestScenario
{
  CPPUNIT_TEST_SUITE( tstMatchMngr );
    CPPUNIT_TEST(testCreateNewGroup);
    CPPUNIT_TEST(testHasGroup);
    CPPUNIT_TEST(testGetGroup);
    //CPPUNIT_TEST(testGetAllPlayers);
    //CPPUNIT_TEST(testRenamePlayer);
//  CPPUNIT_TEST();
//  CPPUNIT_TEST();
//  CPPUNIT_TEST();
//  CPPUNIT_TEST();
//  CPPUNIT_TEST();
  CPPUNIT_TEST_SUITE_END();
  
public:
  void testCreateNewGroup();
  void testHasGroup();
  void testGetGroup();
  //void testGetGroup();
};

#endif	/* TSTGENERICDBOBJECT_H */

