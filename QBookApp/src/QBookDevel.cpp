/*************************************************************************

bq Cervantes e-book reader application
Copyright (C) 2011-2013  Mundoreader, S.L

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the source code.  If not, see <http://www.gnu.org/licenses/>.

*************************************************************************/

//***************************************************************************
//* $Workfile: $
//* $Revision: $
//*
//* Copyright Mundoreader
//***************************************************************************

#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <mntent.h>
#include <net/if.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <dirent.h>
#include <signal.h>
#include <QDateTime>
#include <QBook.h>
#include <PowerManager.h>
#include <PowerManagerLock.h>
#include <hgversion.h>


#include "QBookDevel.h"

PowerManagerLock* QBookDevel::m_powerLock = NULL;

int QBookDevel::getPid(const char *exe) {
    static struct dirent *ent;
    DIR *dir = opendir("/proc");
    if (!dir)
          return 0;
    while( (ent = readdir(dir)) ) {
      if(!ent || !ent->d_name) {
          closedir(dir);
          return 0;
      }
      if(*ent->d_name < '0' || *ent->d_name > '9')
          continue;

      QString *path = new QString();
      path->append("/proc/");
      path->append(ent->d_name);
      path->append("/exe");
      qDebug () << *path;
      QFileInfo f = QFileInfo(*path);

      if (f.isSymLink()) {
           QFileInfo l = QFileInfo (f.symLinkTarget());
            if (l.completeBaseName() == exe) {
                closedir(dir);
                return atoi(ent->d_name);
            }
      }
      // We need to check also cmdline as a lot of binaries are links to busybox
      path = new QString();
      path->append("/proc/");
      path->append(ent->d_name);
      path->append("/cmdline");
      QFile *file = new QFile(*path);
      delete path;
      if (!file->open(QIODevice::ReadOnly))
          continue;

      QTextStream stream(file);
      QString line = stream.readLine();
      file->close();
      delete file;
      QStringList args = line.split((QChar)0); // cmdline is separated with NULLs!
      if (args.size() > 0) {
          f = QFileInfo (args.at(0));
          if (f.completeBaseName() == exe) {
              closedir(dir);
              return atoi(ent->d_name);
          }

      }
    }
    closedir(dir);
    return 0;
}

bool QBookDevel::isUsbNetworkEnabled(void) {
    struct ifaddrs *ifap, *ifa;

    if (getifaddrs(&ifap) != 0)
            return false;

    for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
            if (strcmp (ifa->ifa_name, "usb0") != 0)
                    continue;
            if (ifa->ifa_addr->sa_family == AF_INET && (ifa->ifa_flags & IFF_UP))  {
                freeifaddrs(ifap);
                return true;
            }
    }
    freeifaddrs(ifap);
    return false;

}

bool QBookDevel::isNfsEnabled(void) {
    struct mntent *mp;
    FILE *f = fopen ("/etc/mtab", "r");
    if (f == NULL)
            return false;
    while ( (mp = getmntent (f)) ) {
            if (strcmp(mp->mnt_type, "nfs") == 0) {
                    fclose(f);
                    return true;
            }
    }
    fclose(f);
    return false;

}

bool QBookDevel::isTelnetEnabled(void) {
    return (QBookDevel::getPid("inetd") > 0);
}

bool QBookDevel::areLogsEnabled(void) {
    return (QBookDevel::getPid("rsyslogd") > 0);
}

bool QBookDevel::areCoresEnabled(void) {
    struct rlimit pr;

   if (getrlimit(RLIMIT_CORE, &pr) == -1)  {
       qDebug() << "Cannot get rlimit for cores";
       return false;
   }

   return (pr.rlim_cur > 0);
}

void QBookDevel::enableUsbNetwork(bool enable) {
    qDebug() << Q_FUNC_INFO << enable;

    if (enable)
    {
        if (m_powerLock == NULL)
            QBookDevel::m_powerLock = PowerManager::getNewLock(qApp);

        m_powerLock->activate();
        system("usbup.sh");
    }
    else // Disable
    {
        system("rmmod g_ether");
        system("rmmod arcotg_udc");
        m_powerLock->release();
    }
}

void QBookDevel::enableNfs(void) {
    qDebug() << Q_FUNC_INFO;

    system("nfsup.sh");
}

void QBookDevel::enableTelnet(void) {
    qDebug() << Q_FUNC_INFO;

    system("/usr/sbin/inetd");
}

void QBookDevel::enableLogs(void) {

    QString filter = QBook::getDebugFilter();

    if(filter.isEmpty())
        filter = "*"; // Necessary for regular boot (no restart.sh)

    QBook::setDebug(filter);

    int ret = system("/etc/init.d/rsyslog start");
    qDebug() << "QBookDevel:Enabling logs: " << filter << "result=" << ret ;

}

void QBookDevel::enableCores(void) {
    qDebug() << Q_FUNC_INFO;

    struct rlimit newpr;

    newpr.rlim_cur = 90000000;
    newpr.rlim_max = 90000000;

    if (setrlimit(RLIMIT_CORE, &newpr) == -1)  {
       qDebug() << "Cannot prlimit for cores";
    }

    QString command = QString("echo '/mnt/sd/core.%e.%p.%h.%t_rev") + HG_VERSION + QString("' > /proc/sys/kernel/core_pattern");

    qDebug() << Q_FUNC_INFO << "command" << command;
    char * cmd = command.toLatin1().data();
    system(cmd);

}

void QBookDevel::disableTelnet() {
    qDebug() << Q_FUNC_INFO;

    pid_t pid = QBookDevel::getPid("inetd");
    if (pid > 0)
        kill(pid, SIGKILL);
}

void QBookDevel::disableLogs() {
    qDebug() << Q_FUNC_INFO;

    QBook::setDebug("off");

    qDebug() << Q_FUNC_INFO <<"disableLogs";
    int ret = system("/etc/init.d/rsyslog stop");
    qDebug() << Q_FUNC_INFO <<"disableLogs ret: " << ret ;

}

void QBookDevel::disableCores() {
    qDebug() << Q_FUNC_INFO;

    struct rlimit newpr;

    newpr.rlim_cur = 0;
    newpr.rlim_max = 0;

   if (setrlimit(RLIMIT_CORE, &newpr) == -1)  {
       qDebug() << "Cannot prlimit for cores";
    }
}
