TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS = \
    libukmge \
    shell

TRANSLATIONS += \
    shell/res/i18n/en_US.ts \
    shell/res/i18n/pt_BR.ts \
    shell/res/i18n/zh_CN.ts

isEmpty(PREFIX) {
    PREFIX = /usr
}

qm_files.path = $${PREFIX}/share/openkylinmenueditor/shell/res/i18n/
qm_files.files = shell/res/i18n/*.qm

ts_files.path = $${PREFIX}/share/openkylinmenueditor/shell/res/i18n/
ts_files.files = shell/res/i18n/*.ts

INSTALLS += qm_files ts_files
