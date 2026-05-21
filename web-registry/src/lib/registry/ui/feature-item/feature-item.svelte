<script lang="ts">
	import type { Component } from 'svelte';

	let { Icon, img, label, sublabel, color = 'primary' }: {
		Icon?: Component<{ size?: number; strokeWidth?: number }>;
		img?: string;
		label: string;
		sublabel?: string;
		color?: 'primary' | 'amber';
	} = $props();

	const iconClass = $derived(
		color === 'amber'
			? 'bg-amber-500/10 text-amber-600 dark:text-amber-400'
			: 'bg-primary/10 text-primary'
	);
</script>

<div class="flex items-center gap-3 rounded-xl border border-border bg-card/80 px-4 py-4">
	<div class="flex h-8 w-8 shrink-0 items-center justify-center rounded-lg {iconClass}">
		{#if img}
			<img src={img} alt="" class="h-5 w-5 object-contain" />
		{:else if Icon}
			<Icon size={16} strokeWidth={1.5} />
		{/if}
	</div>
	<div>
		<p class="text-sm font-medium">{label}</p>
		{#if sublabel}
			<p class="text-[10px] font-semibold tracking-wide text-muted-foreground uppercase">{sublabel}</p>
		{/if}
	</div>
</div>
