# locals
locals {
  vendor = module.defaults.tags["Vendor"]

  # [Instance Type]
  aws-vm-size-extra-small = "t3.small"
  aws-vm-size-small       = "t3.medium"
  aws-vm-size-medium      = "t3.large"
  aws-vm-size-large       = "t3.xlarge"
  aws-vm-size-2xlarge     = "t3.2xlarge"

  # [Disk - Volume]
  standard-disk = "gp3"

  # [Disk - Size]
  disk_size = {
    a = {
      ad    = "127"
      ex    = "127"
      iis   = "30"
      desk1 = "127"
      desk2 = "127"
      desk3 = "127"
      tmt   = "127"
    }
    b = {
      ad       = "127"
      ex       = "127"
      file     = "127"
      file-vol = "100"
      sql      = "129"
      kvm      = "127"
      desk1    = "127"
      desk2    = "127"
      desk3    = "127"
      tmt      = "127"
    }
    c = { desk1 = "127" }
    red = {
      kali = "30"
    }
    support = {
      jumpbox   = "127"
      dns       = "127"
      redirect1 = "30"
      redirect2 = "30"
      redirect3 = "30"
      postfix   = "30"
    }
  }

  ####################################################################################
  # Networking Descriptors
  ####################################################################################
  name = {
    vpn = "${var.name-prefix}-vpn-client"
    pcx = "${var.name-prefix}-pcx-range-to-red"

    # network level (VPC)
    alpha = {
      vpc = "${var.name-prefix}-alpha-vpc"
      igw = "${var.name-prefix}-alpha-igw"
      eip = "${var.name-prefix}-alpha-nat-eip"
      nat = "${var.name-prefix}-alpha-nat-gw"
      rtb = "${var.name-prefix}-alpha-rtb"
    }

    beta = {
      vpc = "${var.name-prefix}-beta-vpc"
      igw = "${var.name-prefix}-beta-igw"
      eip = "${var.name-prefix}-beta-nat-eip"
      nat = "${var.name-prefix}-beta-nat-gw"
      rtb = "${var.name-prefix}-beta-rtb"
    }

    # subnet level
    subnet = {
      a = {
        srv  = "${var.name-prefix}-a-srv"
        desk = "${var.name-prefix}-a-desk"
        vpn  = "${var.name-prefix}-a-vpn"
        nat  = "${var.name-prefix}-a-nat"
      }
      b = {
        combined = "${var.name-prefix}-b-combined"
        srv      = "${var.name-prefix}-b-srv"
        desk     = "${var.name-prefix}-b-desk"
      }
      c   = { desk = "${var.name-prefix}-c-desk" }
      red = { main = "${var.name-prefix}-red-main" }
      support = {
        main       = "${var.name-prefix}-support"
        redirect-a = "${var.name-prefix}-support-redirect-a"
        redirect-b = "${var.name-prefix}-support-redirect-b"
        nat        = "${var.name-prefix}-support-nat"
      }
    }

    # machine level (EC2)
    vm = {
      a = {
        ad    = "${var.name-prefix}-a-ad-srv1"
        ex    = "${var.name-prefix}-a-ex-srv1"
        iis   = "${var.name-prefix}-a-iis-srv1"
        desk1 = "${var.name-prefix}-a-desk1"
        desk2 = "${var.name-prefix}-a-desk2"
        desk3 = "${var.name-prefix}-a-desk3"
        tmt   = "${var.name-prefix}-a-tmt"
      }
      b = {
        ad    = "${var.name-prefix}-b-ad-srv1"
        ex    = "${var.name-prefix}-b-ex-srv1"
        file  = "${var.name-prefix}-b-file-srv1"
        sql   = "${var.name-prefix}-b-sql-srv1"
        kvm   = "${var.name-prefix}-b-kvm-srv1"
        desk1 = "${var.name-prefix}-b-desk1"
        desk2 = "${var.name-prefix}-b-desk2"
        desk3 = "${var.name-prefix}-b-desk3"
        tmt   = "${var.name-prefix}-b-tmt"
      }
      c = { desk1 = "${var.name-prefix}-c-desk1" }
      red = {
        kali = "${var.name-prefix}-red-kali"
      }
      support = {
        jumpbox   = "${var.name-prefix}-red-jumpbox"
        dns       = "${var.name-prefix}-support-dns1"
        redirect1 = "${var.name-prefix}-support-redirect1"
        redirect2 = "${var.name-prefix}-support-redirect2"
        redirect3 = "${var.name-prefix}-support-redirect3"
        postfix   = "${var.name-prefix}-support-pf1"
      }
    }

    host = {
      a = {
        ad    = "parrotmon"
        ex    = "omnimon"
        iis   = "gabumon"
        desk1 = "phantomon"
        desk2 = "ghostmon"
        desk3 = "cecilmon"
        tmt   = "amirror"
      }
      b = {
        ad    = "blacknoirmon"
        ex    = "stormfrontmon"
        file  = "alphamon"
        sql   = "datamon"
        kvm   = "leomon"
        desk1 = "kimeramon"
        desk2 = "butchermon"
        desk3 = "bakemon"
        tmt   = "bcmirror"
      }
      c = {
        desk1 = "raremon"
      }
      red = {
        kali = "kraken"
      }
      support = {
        jumpbox   = "homelander"
        dns       = "outofthewoods"
        redirect1 = "gambit"
        redirect2 = "sabretooth"
        redirect3 = "laura"
        postfix   = "deadpool"
      }
    }
  }

  ####################################################################################
  # IP Addressing
  ####################################################################################
  cidr = {
    all_egress = "0.0.0.0/0"

    openvpn = "172.16.0.0/16"

    alpha-all = "10.0.0.0/8"
    alpha     = "10.100.0.0/16"
    alpha-a   = "10.10.0.0/16"
    alpha-b   = "10.20.0.0/16"
    alpha-c   = "10.30.0.0/16"
    alpha-nat = "10.40.0.0/16"
    a = {
      srv  = "10.10.10.0/24"
      desk = "10.10.20.0/24"
      vpn  = "10.10.30.0/24"
    }
    b = {
      combined = "10.20.0.0/16"
      srv      = "10.20.10.0/24"
      desk     = "10.20.20.0/24"
    }
    c          = { desk = "10.30.10.0/24" }
    vendor-nat = "10.40.10.0/24"

    beta       = "176.59.0.0/16"
    beta-nat   = "111.11.0.0/16"
    redirect-a = "121.93.0.0/16"
    redirect-b = "116.83.0.0/16"

    support-red = "176.59.1.0/24"
    support-nat = "111.11.1.0/24"
  }

  alpha_ingress_whitelist = [
    local.cidr.alpha-all,  # self
    local.cidr.redirect-a, # from beta
    local.cidr.redirect-b,
  ]
  beta_ingress_whitelist = [
    local.cidr.beta, # self
    local.cidr.beta-nat,
    local.cidr.redirect-a,
    local.cidr.redirect-b,
    local.cidr.alpha-all, # from alpha
  ]

  ip = {
    # alpha
    a = {
      ad          = "10.10.10.4"
      ex          = "10.10.10.200"
      iis         = "10.10.10.9"
      desk1       = "10.10.20.11"
      desk1-list  = ["10.10.20.101"]
      desk2       = "10.10.20.22"
      desk3       = "10.10.20.23"
      nat-private = "10.10.20.10"
      tmt         = "10.10.10.10"
    }
    b = {
      ad    = "10.20.10.4"
      ex    = "10.20.10.200"
      file  = "10.20.10.23"
      sql   = "10.20.10.122"
      kvm   = "10.20.10.16"
      desk1 = "10.20.20.11"
      desk2 = "10.20.20.22"
      desk3 = "10.20.20.33"
      tmt   = "10.20.20.20"
    }
    c = { desk1 = "10.30.10.4" }

    # beta
    red = {
      kali      = "176.59.1.18"
      kali-list = ["176.59.1.11", "176.59.1.88"]
    }
    support = {
      dns              = "116.83.2.22"
      postfix          = "116.83.2.29"
      nat-private      = "116.83.4.10"
      jumpbox          = "116.83.1.29"
      jumpbox-list     = ["116.83.1.22", "116.83.1.99"]
      redirect-a1      = "121.93.24.2"
      redirect-a1-list = ["121.93.66.49", "121.93.4.32"]
      redirect-a2      = "121.93.99.100"
      redirect-a2-list = ["121.93.44.121"]
      redirect-b       = "116.83.4.99"
      redirect-b-list  = ["116.83.2.91", "116.83.44.32"]
    }
  }
  ####################################################################################
  # RDP Profiles
  ####################################################################################
  # RDP names (outputs.tf)
  a_iis        = ["a-${local.name.host.a.iis}-iis", local.ip.a.iis]
  a_ad         = ["a-${local.name.host.a.ad}-ad", local.ip.a.ad]
  a_ex         = ["a-${local.name.host.a.ex}-ex", local.ip.a.ex]
  a_desk1      = ["a-${local.name.host.a.desk1}-bastion", local.ip.a.desk1]
  a_desk2      = ["a-${local.name.host.a.desk2}-desk2", local.ip.a.desk2]
  a_desk3      = ["a-${local.name.host.a.desk3}-desk3", local.ip.a.desk3]
  b_ad         = ["b-${local.name.host.b.ad}-ad", local.ip.b.ad]
  b_file       = ["b-${local.name.host.b.file}-file", local.ip.b.file]
  b_ex         = ["b-${local.name.host.b.ex}-ex", local.ip.b.ex]
  b_sql        = ["b-${local.name.host.b.sql}-sql", local.ip.b.sql]
  b_kvm        = ["b-${local.name.host.b.kvm}-kvm", local.ip.b.kvm]
  b_desk1      = ["b-${local.name.host.b.desk1}-desk1", local.ip.b.desk1]
  b_desk2      = ["b-${local.name.host.b.desk2}-desk2", local.ip.b.desk2]
  b_desk3      = ["b-${local.name.host.b.desk3}-desk3", local.ip.b.desk3]
  c_desk1      = ["c-${local.name.host.c.desk1}-desk1", local.ip.c.desk1]
  red_kali     = ["evals-red-${local.name.host.red.kali}-kali", local.ip.red.kali]
  red_kali0    = ["evals-red-${local.name.host.red.kali}-kali0", local.ip.red.kali-list[0]]
  red_kali1    = ["evals-red-${local.name.host.red.kali}-kali1", local.ip.red.kali-list[1]]
  sup_jumpbox  = ["evals-support-${local.name.host.support.jumpbox}-jumpbox", local.ip.support.jumpbox]
  sup_jumpbox0 = ["evals-support-${local.name.host.support.jumpbox}-jumpbox0", local.ip.support.jumpbox-list[0]]
  sup_jumpbox1 = ["evals-support-${local.name.host.support.jumpbox}-jumpbox1", local.ip.support.jumpbox-list[1]]
  sup_dns      = ["evals-support-${local.name.host.support.dns}-dns", local.ip.support.dns]
  sup_postfix  = ["evals-support-${local.name.host.support.postfix}-pf", local.ip.support.postfix]
  sup_rd_a1    = ["evals-red-${local.name.host.support.redirect1}-redirect_a1-", local.ip.support.redirect-a1]
  sup_rd_a1_0  = ["evals-red-${local.name.host.support.redirect1}-redirect_a1-0", local.ip.support.redirect-a1-list[0]]
  sup_rd_a1_1  = ["evals-red-${local.name.host.support.redirect1}-redirect_a1-1", local.ip.support.redirect-a1-list[1]]
  sup_rd_a2    = ["evals-red-${local.name.host.support.redirect3}-redirect_a2-", local.ip.support.redirect-a2]
  sup_rd_a2_0  = ["evals-red-${local.name.host.support.redirect3}-redirect_a2-0", local.ip.support.redirect-a2-list[0]]
  sup_rd_b     = ["evals-red-${local.name.host.support.redirect3}-redirect_b", local.ip.support.redirect-b]
  sup_rd_b_0   = ["evals-red-${local.name.host.support.redirect3}-redirect_b0", local.ip.support.redirect-b-list[0]]
  sup_rd_b_1   = ["evals-red-${local.name.host.support.redirect3}-redirect_b1", local.ip.support.redirect-b-list[1]]
}
