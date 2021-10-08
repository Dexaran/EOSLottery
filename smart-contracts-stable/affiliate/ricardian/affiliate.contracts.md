<h1 class="contract">init</h1>
---
spec_version: 0.2.0
title: Contract Initialization
summary: Predefine initial state of the {{$action.account}} contract.
icon: https://upload.wikimedia.org/wikipedia/commons/thumb/4/44/Attraction_transfer_icon.svg/200px-Attraction_transfer_icon.svg.png#24785AF89B4480048CF951E39D0C3D70EE20C44F66582F0890938378C0025064
---

I certify the following to be true to the best of my knowledge:
0. I have all the necessary rights and powers to perform this action.
1. This action does not violate the terms of the {{$action.account}} contract.
2. Proposed locked amount is equal to {{locked}} and not proceeds of fraudulent or violent activities.
3. Proposed witness is equal to {{to_json witness}}, while private key for it is acknowledged and stored securely enough.
4. Proposed TTLs are equal to: {{to_json ttl}}

{{$clauses.agreement}}

{{$clauses.amendment}}

{{$clauses.warranty}}

{{$clauses.force_majeure}}

{{$clauses.irreversibility_notice}}

{{$clauses.authorized_signature}}


<h1 class="contract">config</h1>
---
spec_version: 0.2.0
title: Contract Configuration
summary: Modify configuration of the {{$action.account}} contract.
icon: https://upload.wikimedia.org/wikipedia/commons/thumb/4/44/Attraction_transfer_icon.svg/200px-Attraction_transfer_icon.svg.png#24785AF89B4480048CF951E39D0C3D70EE20C44F66582F0890938378C0025064
---

I certify the following to be true to the best of my knowledge:
0. I have all the necessary rights and powers to perform this action.
1. This action does not violate the terms of the {{$action.account}} contract.
2. Proposed TTLs are equal to: {{to_json ttl}}

{{$clauses.agreement}}

{{$clauses.amendment}}

{{$clauses.warranty}}

{{$clauses.force_majeure}}

{{$clauses.irreversibility_notice}}

{{$clauses.authorized_signature}}


<h1 class="contract">rotate</h1>
---
spec_version: 0.2.0
title: Witness Rotation
summary: Modify witness of the {{$action.account}} contract.
icon: https://upload.wikimedia.org/wikipedia/commons/thumb/4/44/Attraction_transfer_icon.svg/200px-Attraction_transfer_icon.svg.png#24785AF89B4480048CF951E39D0C3D70EE20C44F66582F0890938378C0025064
---

I certify the following to be true to the best of my knowledge:
0. I have all the necessary rights and powers to perform this action.
1. This action does not violate the terms of the {{$action.account}} contract.
2. Proposed witness equal to {{to_json witness}}, while private key for it is acknowledged and stored securely enough.

{{$clauses.agreement}}

{{$clauses.amendment}}

{{$clauses.warranty}}

{{$clauses.force_majeure}}

{{$clauses.irreversibility_notice}}

{{$clauses.authorized_signature}}


<h1 class="contract">evacuate</h1>
---
spec_version: 0.2.0
title: Emergency Evacuation
summary: Emergency evacuate {{$action.account}} contract.
icon: https://upload.wikimedia.org/wikipedia/commons/thumb/4/44/Attraction_transfer_icon.svg/200px-Attraction_transfer_icon.svg.png#24785AF89B4480048CF951E39D0C3D70EE20C44F66582F0890938378C0025064
---

I certify the following to be true to the best of my knowledge:
0. I have all the necessary rights and powers to perform this action.
1. The reason is quite serious and does not violate the terms of the {{$action.account}} contract.
2. EOS (if any) will be transferred to a spare account and contract will be in a locked state.

{{$clauses.agreement}}

{{$clauses.amendment}}

{{$clauses.warranty}}

{{$clauses.force_majeure}}

{{$clauses.irreversibility_notice}}

{{$clauses.authorized_signature}}


<h1 class="contract">announce</h1>
---
spec_version: 0.2.0
title: License Announcement
summary: Announce new license of the {{$action.account}} contract.
icon: https://upload.wikimedia.org/wikipedia/commons/thumb/4/44/Attraction_transfer_icon.svg/200px-Attraction_transfer_icon.svg.png#24785AF89B4480048CF951E39D0C3D70EE20C44F66582F0890938378C0025064
---

I certify the following to be true to the best of my knowledge:
0. I have all the necessary rights and powers to perform this action.
1. This action does not violate the terms of the {{$action.account}} contract.
2. Proposed license name is equal to "{{license}}".
3. Proposed license fee is equal to {{fee}}.
4. Proposed license rate is equal to: {{to_json rate}}
5. Proposed license terms is equal to: {{to_json terms}}

{{$clauses.agreement}}

{{$clauses.amendment}}

{{$clauses.warranty}}

{{$clauses.force_majeure}}

{{$clauses.irreversibility_notice}}

{{$clauses.authorized_signature}}


<h1 class="contract">activate</h1>
---
spec_version: 0.2.0
title: License Activation
summary: Activate existing license of the {{$action.account}} contract.
icon: https://upload.wikimedia.org/wikipedia/commons/thumb/4/44/Attraction_transfer_icon.svg/200px-Attraction_transfer_icon.svg.png#24785AF89B4480048CF951E39D0C3D70EE20C44F66582F0890938378C0025064
---

I certify the following to be true to the best of my knowledge:
0. I have all the necessary rights and powers to perform this action.
1. This action does not violate the terms of the {{$action.account}} contract.
2. Proposed license id is equal to {{license}}.

{{$clauses.agreement}}

{{$clauses.amendment}}

{{$clauses.warranty}}

{{$clauses.force_majeure}}

{{$clauses.irreversibility_notice}}

{{$clauses.authorized_signature}}


<h1 class="contract">allocate</h1>
---
spec_version: 0.2.0
title: Partner Allocation
summary: Allocate new partner of the {{$action.account}} contract.
icon: https://upload.wikimedia.org/wikipedia/commons/thumb/4/44/Attraction_transfer_icon.svg/200px-Attraction_transfer_icon.svg.png#24785AF89B4480048CF951E39D0C3D70EE20C44F66582F0890938378C0025064
---

I certify the following to be true to the best of my knowledge:
0. I have all the necessary rights and powers to perform this action.
1. This action does not violate the terms of the {{$action.account}} contract.
2. Proposed partner account name is equal to "{{partner}}".
{{#if_has_value affiliate}}
3. Proposed partner affiliate name is equal to "{{affiliate}}".
{{ else }}
3. Proposed partner affiliate name is not set.
{{/if_has_value}}
4. Proposed partner license id is equal to {{license}}.
{{#if_has_value rate}}
5. Proposed partner rate is equal to {{rate}}.
{{ else }}
5. Proposed partner rate is not set.
{{/if_has_value}}
{{#if_has_value dealer}}
6. Proposed partner dealer is equal to: {{to_json dealer}}
{{ else }}
6. Proposed partner dealer is not set.
{{/if_has_value}}
7. Proposed partner discount is equal to {{discount}}.
8. Proposed partner approved flag is equal to {{approved}}.

{{$clauses.agreement}}

{{$clauses.amendment}}

{{$clauses.warranty}}

{{$clauses.force_majeure}}

{{$clauses.irreversibility_notice}}

{{$clauses.authorized_signature}}


<h1 class="contract">empower</h1>
---
spec_version: 0.2.0
title: Entity Capabilities Extension
summary: Empower existing object scope of the {{$action.account}} contract.
icon: https://upload.wikimedia.org/wikipedia/commons/thumb/4/44/Attraction_transfer_icon.svg/200px-Attraction_transfer_icon.svg.png#24785AF89B4480048CF951E39D0C3D70EE20C44F66582F0890938378C0025064
---

I certify the following to be true to the best of my knowledge:
0. I have all the necessary rights and powers to perform this action.
1. The reason is quite serious and does not violate the terms of the {{$action.account}} contract.
2. Proposed object is equal to: {{to_json object}}
3. Proposed object scope id is equal to {{scope}}.

{{$clauses.agreement}}

{{$clauses.amendment}}

{{$clauses.warranty}}

{{$clauses.force_majeure}}

{{$clauses.irreversibility_notice}}

{{$clauses.authorized_signature}}


<h1 class="contract">revoke</h1>
---
spec_version: 0.2.0
title: Entity Capabilities Reduction
summary: Revoke existing object scope of the {{$action.account}} contract.
icon: https://upload.wikimedia.org/wikipedia/commons/thumb/4/44/Attraction_transfer_icon.svg/200px-Attraction_transfer_icon.svg.png#24785AF89B4480048CF951E39D0C3D70EE20C44F66582F0890938378C0025064
---

I certify the following to be true to the best of my knowledge:
0. I have all the necessary rights and powers to perform this action.
1. The reason is quite serious and does not violate the terms of the {{$action.account}} contract.
2. Proposed object is equal to: {{to_json object}}
3. Proposed object scope id is equal to {{scope}}.

{{$clauses.agreement}}

{{$clauses.amendment}}

{{$clauses.warranty}}

{{$clauses.force_majeure}}

{{$clauses.irreversibility_notice}}

{{$clauses.authorized_signature}}


<h1 class="contract">archive</h1>
---
spec_version: 0.2.0
title: License Archiving
summary: Archive existing license of the {{$action.account}} contract.
icon: https://upload.wikimedia.org/wikipedia/commons/thumb/4/44/Attraction_transfer_icon.svg/200px-Attraction_transfer_icon.svg.png#24785AF89B4480048CF951E39D0C3D70EE20C44F66582F0890938378C0025064
---

I certify the following to be true to the best of my knowledge:
0. I have all the necessary rights and powers to perform this action.
1. The reason is quite serious and does not violate the terms of the {{$action.account}} contract.
2. Proposed license id is equal to {{license}}.

{{$clauses.agreement}}

{{$clauses.amendment}}

{{$clauses.warranty}}

{{$clauses.force_majeure}}

{{$clauses.irreversibility_notice}}

{{$clauses.authorized_signature}}


<h1 class="contract">suspend</h1>
---
spec_version: 0.2.0
title: Partnership Suspension
summary: Suspend existing partner of the {{$action.account}} contract.
icon: https://upload.wikimedia.org/wikipedia/commons/thumb/4/44/Attraction_transfer_icon.svg/200px-Attraction_transfer_icon.svg.png#24785AF89B4480048CF951E39D0C3D70EE20C44F66582F0890938378C0025064
---

I certify the following to be true to the best of my knowledge:
0. I have all the necessary rights and powers to perform this action.
1. The reason is quite serious and does not violate the terms of the {{$action.account}} contract.
2. Proposed partner account name is equal to {{partner}}.
{{#if_has_value deal}}
3. Proposed partner deal id is equal to "{{deal}}".
{{ else }}
3. Proposed partner deal id is not set.
{{/if_has_value}}

{{$clauses.agreement}}

{{$clauses.amendment}}

{{$clauses.warranty}}

{{$clauses.force_majeure}}

{{$clauses.irreversibility_notice}}

{{$clauses.authorized_signature}}


<h1 class="contract">resume</h1>
---
spec_version: 0.2.0
title: Partnership Renewal
summary: Resume existing partner of the {{$action.account}} contract.
icon: https://upload.wikimedia.org/wikipedia/commons/thumb/4/44/Attraction_transfer_icon.svg/200px-Attraction_transfer_icon.svg.png#24785AF89B4480048CF951E39D0C3D70EE20C44F66582F0890938378C0025064
---

I certify the following to be true to the best of my knowledge:
0. I have all the necessary rights and powers to perform this action.
1. The reason is quite serious and does not violate the terms of the {{$action.account}} contract.
2. Proposed partner account name is equal to {{partner}}.
{{#if_has_value deal}}
3. Proposed partner deal id is equal to "{{deal}}".
{{ else }}
3. Proposed partner deal id is not set.
{{/if_has_value}}

{{$clauses.agreement}}

{{$clauses.amendment}}

{{$clauses.warranty}}

{{$clauses.force_majeure}}

{{$clauses.irreversibility_notice}}

{{$clauses.authorized_signature}}


<h1 class="contract">sync</h1>
---
spec_version: 0.2.0
title: Accounts Synchronization
summary: Sync existing accounts of the {{$action.account}} contract.
icon: https://upload.wikimedia.org/wikipedia/commons/thumb/4/44/Attraction_transfer_icon.svg/200px-Attraction_transfer_icon.svg.png#24785AF89B4480048CF951E39D0C3D70EE20C44F66582F0890938378C0025064
---

I certify the following to be true to the best of my knowledge:
0. I have all the necessary rights and powers to perform this action.
1. This action does not violate the terms of the {{$action.account}} contract.
2. The accuracy of the calculations is confirmed.

{{$clauses.agreement}}

{{$clauses.amendment}}

{{$clauses.warranty}}

{{$clauses.force_majeure}}

{{$clauses.irreversibility_notice}}

{{$clauses.authorized_signature}}


<h1 class="contract">apply</h1>
---
spec_version: 0.2.0
title: Partnership Applyment
summary: Apply for a new partner of the {{$action.account}} contract.
icon: https://upload.wikimedia.org/wikipedia/commons/thumb/4/44/Attraction_transfer_icon.svg/200px-Attraction_transfer_icon.svg.png#24785AF89B4480048CF951E39D0C3D70EE20C44F66582F0890938378C0025064
---

I certify the following to be true to the best of my knowledge:
0. I have all the necessary rights and powers to perform this action.
1. This action does not violate the terms of the {{$action.account}} contract.
2. Proposed partner account name is equal to "{{account}}".
3. Proposed partner license id is equal to {{license}}.
4. Proposed partner rate is equal to {{rate}}.
5. Proposed partner dealer is equal to: {{to_json dealer}}
6. Proposed partner discount is equal to {{discount}}.

{{$clauses.agreement}}

{{$clauses.amendment}}

{{$clauses.warranty}}

{{$clauses.force_majeure}}

{{$clauses.irreversibility_notice}}

{{$clauses.authorized_signature}}


<h1 class="contract">refuse</h1>
---
spec_version: 0.2.0
title: Application Removal
summary: Refuse existing application from the {{$action.account}} contract.
icon: https://upload.wikimedia.org/wikipedia/commons/thumb/4/44/Attraction_transfer_icon.svg/200px-Attraction_transfer_icon.svg.png#24785AF89B4480048CF951E39D0C3D70EE20C44F66582F0890938378C0025064
---

I certify the following to be true to the best of my knowledge:
0. I have all the necessary rights and powers to perform this action.
1. The reason is quite serious and does not violate the terms of the {{$action.account}} contract.
2. Proposed application id is equal to {{application}}.

{{$clauses.agreement}}

{{$clauses.amendment}}

{{$clauses.warranty}}

{{$clauses.force_majeure}}

{{$clauses.irreversibility_notice}}

{{$clauses.authorized_signature}}


<h1 class="contract">claim</h1>
---
spec_version: 0.2.0
title: Application Execution
summary: Claim partnership of the {{$action.account}} contract.
icon: https://upload.wikimedia.org/wikipedia/commons/thumb/4/44/Attraction_transfer_icon.svg/200px-Attraction_transfer_icon.svg.png#24785AF89B4480048CF951E39D0C3D70EE20C44F66582F0890938378C0025064
---

I certify the following to be true to the best of my knowledge:
0. I have all the necessary rights and powers to perform this action.
1. This action does not violate the terms of the {{$action.account}} contract.
2. Proposed partner account name is equal to {{partner}}.

{{$clauses.agreement}}

{{$clauses.amendment}}

{{$clauses.warranty}}

{{$clauses.force_majeure}}

{{$clauses.irreversibility_notice}}

{{$clauses.authorized_signature}}


<h1 class="contract">prolong</h1>
---
spec_version: 0.2.0
title: Partner Account Prolongation
summary: Reset partner safeguard timer of the {{$action.account}} contract.
icon: https://upload.wikimedia.org/wikipedia/commons/thumb/4/44/Attraction_transfer_icon.svg/200px-Attraction_transfer_icon.svg.png#24785AF89B4480048CF951E39D0C3D70EE20C44F66582F0890938378C0025064
---

I certify the following to be true to the best of my knowledge:
0. I have all the necessary rights and powers to perform this action.
1. This action does not violate the terms of the {{$action.account}} contract.
2. Proposed partner account name is equal to {{partner}}.

{{$clauses.agreement}}

{{$clauses.amendment}}

{{$clauses.warranty}}

{{$clauses.force_majeure}}

{{$clauses.irreversibility_notice}}

{{$clauses.authorized_signature}}


<h1 class="contract">withdraw</h1>
---
spec_version: 0.2.0
title: Partner Funds Withdrawal
summary: Withdraw partner's {{quantity}} from the {{$action.account}} contract.
icon: https://upload.wikimedia.org/wikipedia/commons/thumb/4/44/Attraction_transfer_icon.svg/200px-Attraction_transfer_icon.svg.png#24785AF89B4480048CF951E39D0C3D70EE20C44F66582F0890938378C0025064
---

I certify the following to be true to the best of my knowledge:
0. I have all the necessary rights and powers to perform this action.
1. This action does not violate the terms of the {{$action.account}} contract.
2. Proposed partner account name is equal to {{partner}}.
3. Proposed payee account name is equal to {{payee}}.
4. Proposed quantity is equal to {{quantity}} and is not proceeds of fraudulent or violent activities.
5. Any contractual terms & conditions with respect to {{quantity}} (if any) was disclosed to {{payee}}.

{{$clauses.agreement}}

{{$clauses.amendment}}

{{$clauses.warranty}}

{{$clauses.force_majeure}}

{{$clauses.irreversibility_notice}}

{{$clauses.authorized_signature}}


<h1 class="contract">update</h1>
---
spec_version: 0.2.0
title: Partner Update
summary: Update existing partner of the {{$action.account}} contract.
icon: https://upload.wikimedia.org/wikipedia/commons/thumb/4/44/Attraction_transfer_icon.svg/200px-Attraction_transfer_icon.svg.png#24785AF89B4480048CF951E39D0C3D70EE20C44F66582F0890938378C0025064
---

I certify the following to be true to the best of my knowledge:
0. I have all the necessary rights and powers to perform this action.
1. This action does not violate the terms of the {{$action.account}} contract.
2. Proposed partner account name is equal to {{partner}}.
3. Proposed license id is equal to {{license}}.
{{#if_has_value rate}}
4. Proposed new license rate is equal to {{rate}}.
{{ else }}
4. Proposed new license rate is not set.
{{/if_has_value}}
{{#if_has_value rate}}
5. Proposed new partner balance is equal to {{balance}}.
{{ else }}
5. Proposed new partner balance is not set.
{{/if_has_value}}

{{$clauses.agreement}}

{{$clauses.amendment}}

{{$clauses.warranty}}

{{$clauses.force_majeure}}

{{$clauses.irreversibility_notice}}

{{$clauses.authorized_signature}}


<h1 class="contract">upgrade</h1>
---
spec_version: 0.2.0
title: License Upgrade
summary: Upgrade existing partner license of the {{$action.account}} contract.
icon: https://upload.wikimedia.org/wikipedia/commons/thumb/4/44/Attraction_transfer_icon.svg/200px-Attraction_transfer_icon.svg.png#24785AF89B4480048CF951E39D0C3D70EE20C44F66582F0890938378C0025064
---

I certify the following to be true to the best of my knowledge:
0. I have all the necessary rights and powers to perform this action.
1. This action does not violate the terms of the {{$action.account}} contract.
2. Proposed partner account name is equal to {{partner}}.
3. Proposed current license id is equal to {{license.first}}.
4. Proposed new license id is equal to {{license.second}}.
{{#if_has_value rate}}
5. Proposed new license rate is equal to {{rate}}.
{{ else }}
5. Proposed new license rate is not set.
{{/if_has_value}}

{{$clauses.agreement}}

{{$clauses.amendment}}

{{$clauses.warranty}}

{{$clauses.force_majeure}}

{{$clauses.irreversibility_notice}}

{{$clauses.authorized_signature}}


<h1 class="contract">insure</h1>
---
spec_version: 0.2.0
title: Partner Access Insurance
summary: Insure existing partner of the {{$action.account}} contract.
icon: https://upload.wikimedia.org/wikipedia/commons/thumb/4/44/Attraction_transfer_icon.svg/200px-Attraction_transfer_icon.svg.png#24785AF89B4480048CF951E39D0C3D70EE20C44F66582F0890938378C0025064
---

I certify the following to be true to the best of my knowledge:
0. I have all the necessary rights and powers to perform this action.
1. This action does not violate the terms of the {{$action.account}} contract.
2. Proposed partner account name is equal to {{partner}}.
3. Proposed standby account name is equal to {{standby}}.

{{$clauses.agreement}}

{{$clauses.amendment}}

{{$clauses.warranty}}

{{$clauses.force_majeure}}

{{$clauses.irreversibility_notice}}

{{$clauses.authorized_signature}}


<h1 class="contract">failover</h1>
---
spec_version: 0.2.0
title: Partner Access Recovery
summary: Failover for existing partner of the {{$action.account}} contract.
icon: https://upload.wikimedia.org/wikipedia/commons/thumb/4/44/Attraction_transfer_icon.svg/200px-Attraction_transfer_icon.svg.png#24785AF89B4480048CF951E39D0C3D70EE20C44F66582F0890938378C0025064
---

I certify the following to be true to the best of my knowledge:
0. I have all the necessary rights and powers to perform this action.
1. This action does not violate the terms of the {{$action.account}} contract.
2. Proposed partner account name is equal to {{partner}}.

{{$clauses.agreement}}

{{$clauses.amendment}}

{{$clauses.warranty}}

{{$clauses.force_majeure}}

{{$clauses.irreversibility_notice}}

{{$clauses.authorized_signature}}


<h1 class="contract">delegate</h1>
---
spec_version: 0.2.0
title: Partner Authority Delegation
summary: Delegate authority of existing partner of the {{$action.account}} contract.
icon: https://upload.wikimedia.org/wikipedia/commons/thumb/4/44/Attraction_transfer_icon.svg/200px-Attraction_transfer_icon.svg.png#24785AF89B4480048CF951E39D0C3D70EE20C44F66582F0890938378C0025064
---

I certify the following to be true to the best of my knowledge:
0. I have all the necessary rights and powers to perform this action.
1. This action does not violate the terms of the {{$action.account}} contract.
2. Proposed partner account name is equal to {{partner}}.
3. Proposed successor account name is equal to {{successor}}.

{{$clauses.agreement}}

{{$clauses.amendment}}

{{$clauses.warranty}}

{{$clauses.force_majeure}}

{{$clauses.irreversibility_notice}}

{{$clauses.authorized_signature}}


<h1 class="contract">quit</h1>
---
spec_version: 0.2.0
title: Partner Self-Dismissal
summary: Leave partnership of the {{$action.account}} contract.
icon: https://upload.wikimedia.org/wikipedia/commons/thumb/4/44/Attraction_transfer_icon.svg/200px-Attraction_transfer_icon.svg.png#24785AF89B4480048CF951E39D0C3D70EE20C44F66582F0890938378C0025064
---

I certify the following to be true to the best of my knowledge:
0. I have all the necessary rights and powers to perform this action.
1. The reason is quite serious and does not violate the terms of the {{$action.account}} contract.
2. Proposed partner account name is equal to {{partner}}.

{{$clauses.agreement}}

{{$clauses.amendment}}

{{$clauses.warranty}}

{{$clauses.force_majeure}}

{{$clauses.irreversibility_notice}}

{{$clauses.authorized_signature}}


<h1 class="contract">migrate</h1>
---
spec_version: 0.2.0
title: Contract Migration
summary: Execute specific version migrations of the {{$action.account}} contract.
icon: https://upload.wikimedia.org/wikipedia/commons/thumb/4/44/Attraction_transfer_icon.svg/200px-Attraction_transfer_icon.svg.png#24785AF89B4480048CF951E39D0C3D70EE20C44F66582F0890938378C0025064
---

I certify the following to be true to the best of my knowledge:
0. I have all the necessary rights and powers to perform this action.
1. This action does not violate the terms of the {{$action.account}} contract.
2. Proposed version is equal to {{version}}.

{{$clauses.agreement}}

{{$clauses.amendment}}

{{$clauses.warranty}}

{{$clauses.force_majeure}}

{{$clauses.irreversibility_notice}}

{{$clauses.authorized_signature}}