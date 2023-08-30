## -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

def build(bld):
    module = bld.create_ns3_module('dmsrp', ['internet', 'wifi'])
    module.includes = '.'
    module.source = [
        'model/dmsrp-rtable.cc',
        'model/dmsrp-packet.cc',
        'model/dmsrp-routing-protocol.cc',
        'helper/dmsrp-helper.cc',

        ]

    dmsrp_test = bld.create_ns3_module_test_library('dmsrp')
    dmsrp_test.source = [
        'test/dmsrp-test-suite.cc',
        'test/loopback.cc',
        ]

    headers = bld(features='ns3header')
    headers.module = 'dmsrp'
    headers.source = [
        'model/dmsrp-rtable.h',
        'model/dmsrp-packet.h',
        'model/dmsrp-routing-protocol.h',
        'helper/dmsrp-helper.h',
        ]

    if bld.env['ENABLE_EXAMPLES']:
        bld.recurse('examples')

   # bld.ns3_python_bindings()
