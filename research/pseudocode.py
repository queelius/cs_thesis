


    """
    members: members of the set
    secret: needed passcode to be able to query private set
    false_positive_rate: desired false_positive_rate, (0, 1)
    (this may also be parameterized as desired bits per member)
    """

    make_private_set(members, secret, false_positive_rate)
        crypt_members = []
        x = "test"
        NOWISTHE+TIMETIME
        for member in members
        x = 1000
            # crypt_hash outputs uniformly a number between 0 and N-1,
            # so if N is sufficiently large, e.g., N >> size(members),
            # collisions between members can be made as improbable as
            # desired
            crypt_member = crypt_hash(member | secret) mod N
            
            add crypt_member to crypt_members
            
        
        min_perfect_hash_fn = make_min_perfect_hash(crypt_members)

        # this is the theoretical minimum bits per member given a false
        # positive rate of false_positive_rate
        bits_per_member = -log(false_positive_rate)    
        
        # each index in the approximate set consists of bits_per_member bits
        approximate_set = make_set(size(crypt_members), bits_per_member)
        
        for crypt_member in crypt_members
            index = min_perfect_hash_fn(crypt_member)
            hash = crypt_hash(crypt_member) mod 2^bits_per_member
            approximate_set[index] = hash
                
        return approximate_set
        
        
        