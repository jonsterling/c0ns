CC = cc0
DEPS = bit_utils.c0 c0ns.c1 class.c0 domain_name.c0 header.c0 message.c1 \
question.c0 resource_record.c1 types.c0

checked: $(DEPS)
	$(CC) -d -o c0ns_checked c0ns.c1

unchecked: $(DEPS)
	$(CC) -o c0ns c0ns.c1
