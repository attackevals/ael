using Microsoft.VisualStudio.TestTools.UnitTesting;
using Quasar.Client.Messages;
using System;
using System.Collections.Generic;
using System.Net;

namespace Quasar.Client.Tests
{
    [TestClass]
    public class ClientTests
    {
        [TestMethod]
        public void TestGetTargetAddrListValid()
        {
            string input1 = "10.1.2.0/24";
            var result = PortScanHandler.GetTargetAddrList(input1);
            Assert.AreEqual(result.Count, 256);
            Assert.AreEqual(result[0], IPAddress.Parse("10.1.2.0"));
            Assert.AreEqual(result[1], IPAddress.Parse("10.1.2.1"));
            Assert.AreEqual(result[100], IPAddress.Parse("10.1.2.100"));
            Assert.AreEqual(result[255], IPAddress.Parse("10.1.2.255"));

            string input2 = "10.1.2.2/29";
            result = PortScanHandler.GetTargetAddrList(input2);
            Assert.AreEqual(result.Count, 8);
            Assert.AreEqual(result[0], IPAddress.Parse("10.1.2.0"));
            Assert.AreEqual(result[1], IPAddress.Parse("10.1.2.1"));
            Assert.AreEqual(result[2], IPAddress.Parse("10.1.2.2"));
            Assert.AreEqual(result[3], IPAddress.Parse("10.1.2.3"));
            Assert.AreEqual(result[4], IPAddress.Parse("10.1.2.4"));
            Assert.AreEqual(result[5], IPAddress.Parse("10.1.2.5"));
            Assert.AreEqual(result[6], IPAddress.Parse("10.1.2.6"));
            Assert.AreEqual(result[7], IPAddress.Parse("10.1.2.7"));
        }

        [TestMethod]
        public void TestGetTargetAddrListInvalid()
        {
            try
            {
                var result = PortScanHandler.GetTargetAddrList("10.1.2.0/33");
                Assert.Fail();
            }
            catch (Exception ex)
            {
                Assert.AreEqual("CIDR mask 33 either too large or too small", ex.Message);
            }

            try
            {
                var result = PortScanHandler.GetTargetAddrList("10.1.2.0/15");
                Assert.Fail();
            }
            catch (Exception ex)
            {
                Assert.AreEqual("CIDR mask 15 either too large or too small", ex.Message);
            }

            try
            {
                var result = PortScanHandler.GetTargetAddrList("10.1.2.3.4/20");
                Assert.Fail();
            }
            catch (Exception ex)
            {
                Assert.AreEqual("Invalid CIDR notation: 10.1.2.3.4/20", ex.Message);
            }

            try
            {
                var result = PortScanHandler.GetTargetAddrList("avcd");
                Assert.Fail();
            }
            catch (Exception ex)
            {
                Assert.AreEqual("Invalid CIDR notation: avcd", ex.Message);
            }
        }
    }
}
