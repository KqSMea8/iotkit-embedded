#! /bin/bash

TARGET_FILE=$1
rm -f ${TARGET_FILE}

IFLAGS=$( \
for iter in \
    $(find -L ${TOP_DIR} -type d -not -path "*.git*" -not -path "*.O*"); do \
        echo "    -I${iter} \\"; \
done)
ALL_LIBS=$(for iter in ${ALL_LIBS}; do echo -n "${OUTPUT_DIR}/usr/lib/${iter} "; done)
ALL_BINS=$(for iter in ${ALL_PROG}; do echo -n "${OUTPUT_DIR}/usr/bin/${iter} "; done)

cat << EOB >> ${TARGET_FILE}
include ${RULE_DIR}/funcs.mk

VPATH := $(for iter in ${COMP_LIB_COMPONENTS}; do echo -n "${OUTPUT_DIR}/${iter} "; done)

.PHONY: all
all: ${OUTPUT_DIR}/usr/lib/${COMP_LIB} ${ALL_LIBS} ${ALL_BINS}

${OUTPUT_DIR}/usr/lib/${COMP_LIB}: \\
$(for iter in ${COMP_LIB_OBJS}; do
    echo "    ${iter} \\"
done
)

	@mkdir -p \$\$(dirname \$@)
	@\$(call Brief_Log,"AR",\$\$(basename \$@),"...")
	@${AR} -rcs \$@ \$^

%.o:
	@\$(call Brief_Log,"CC",\$\$(basename \$@),"...")
	@mkdir -p \$\$(dirname \$@)
	@S=\$\$(echo \$@|sed 's:${OUTPUT_DIR}:${TOP_DIR}:1'); \\
    ${CC} -c \\
        -o \$@ \\
        ${CFLAGS} \\
        ${IFLAGS}
        \$\${S//.o/.c}

EOB

for i in ${ALL_LIBS}; do
    n=$(basename ${i})
    j=$(grep "${n}$" ${STAMP_BLD_VAR}|cut -d' ' -f1|sed 's:LIBA_TARGET_::1')
    k=$(echo 'LIB_OBJS_'"${j}")
    k=$(grep -m 1 "^${k}" ${STAMP_BLD_VAR}|cut -d' ' -f3-)
    k=$(for l in ${k}; do echo -n "${OUTPUT_DIR}/${j}/${l} "; done)

    cat << EOB >> ${TARGET_FILE}
${OUTPUT_DIR}/usr/lib/${n}: \\
$(for m in ${k}; do
    echo "    ${m} \\";
done)

	@mkdir -p \$\$(dirname \$@)
	@\$(call Brief_Log,"AR",\$\$(basename \$@),"...")
	@${AR} -rcs \$@ \$^

EOB
done

k=""
for i in ${ALL_PROG}; do
    j=$(grep -m 1 "^SRCS_${i}" ${STAMP_BLD_VAR}|cut -d' ' -f3-)
    if [ "${k}" = "" ]; then
        k=$(grep -m 1 "TARGET_.* = .*${i}" ${STAMP_BLD_VAR}|cut -d' ' -f1|sed 's:TARGET_::1')
        LFLAGS=$(grep -m 1 "^LDFLAGS_${k}" ${STAMP_BLD_VAR}|cut -d' ' -f3-)
    fi

    cat << EOB >> ${TARGET_FILE}
${OUTPUT_DIR}/usr/bin/${i}: \\
$(for k in ${j} ${OUTPUT_DIR}/usr/lib/${COMP_LIB} ${ALL_LIBS}; do
    echo "    ${k} \\";
done)

	@\$(call Brief_Log,"LD",\$\$(basename \$@),"...")
	@${CC} \\
        -o \$@ \\
        ${IFLAGS}
        ${CFLAGS} \\
        ${LFLAGS} \\
        -L${OUTPUT_DIR}/usr/lib \\
        \$^

EOB
done