/*
 * Copyright © 2014 Volker Knollmann
 * 
 * This work is free. You can redistribute it and/or modify it under the
 * terms of the Do What The Fuck You Want To Public License, Version 2,
 * as published by Sam Hocevar. See the COPYING file or visit
 * http://www.wtfpl.net/ for more details.
 * 
 * This program comes without any warranty. Use it at your own risk or
 * don't use it at all.
 */

#include <cstdlib>
#include <QApplication>
#include <cppunit/TestSuite.h>
#include <cppunit/ui/text/TestRunner.h>

#include "tstGenericDBObject.h"
#include "tstTournament.h"
#include "tstTeamMngr.h"
#include "tstTeam.h"
#include "tstTeamMngr.h"
#include "tstCatMngr.h"

// comment / uncomment the following to execute just one suite
//#define TEST_ALL_SUITES

// Registers the fixture into the 'registry'
#ifdef TEST_ALL_SUITES
  CPPUNIT_TEST_SUITE_REGISTRATION(tstGenericDBObject);
  CPPUNIT_TEST_SUITE_REGISTRATION(tstTournament);
  CPPUNIT_TEST_SUITE_REGISTRATION(tstTeamMngr);
  CPPUNIT_TEST_SUITE_REGISTRATION(tstTeam);
  CPPUNIT_TEST_SUITE_REGISTRATION(tstCatMngr);
#else
  CPPUNIT_TEST_SUITE_REGISTRATION(tstTeam);
#endif


int main(int argc, char *argv[]) {
    // initialize resources, if needed
    // Q_INIT_RESOURCE(resfile);

    QApplication app(argc, argv);
    
    CppUnit::TextUi::TestRunner runner;
    CppUnit::Test *suite;
    
    suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();
    
    runner.addTest(suite);
    
    runner.run();
    
}
