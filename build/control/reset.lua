function install()
  runsql("create table `source`(`sname` text,`path` text)")
  runsql("create table `notes`(`sname` text,`path` text)")
end